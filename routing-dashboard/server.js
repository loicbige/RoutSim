const dgram = require('dgram');
const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const path = require('path');

// Configuration
const UDP_PORT = 8080;
const HTTP_PORT = 3000;

// Initialisation
const app = express();
const server = http.createServer(app);
const io = socketIo(server);
const udpServer = dgram.createSocket('udp4');

// État du réseau
const networkState = {
  nodes: new Map(), // routeurs: {id, ip, lastSeen}
  links: new Map()  // liens: {id, from, to, status, lastUpdate} (clé canonique non orientée)
};

// Servir les fichiers statiques
app.use(express.static('public'));

app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Gestion des connexions Socket.io
io.on('connection', (socket) => {
  console.log('✓ Client Web connecté:', socket.id);

  // Envoyer l'état actuel au nouveau client
  socket.emit('initial-state', {
    nodes: Array.from(networkState.nodes.values()),
    links: Array.from(networkState.links.values())
  });

  socket.on('disconnect', () => {
    console.log('✗ Client Web déconnecté:', socket.id);
  });
});

/* ===========================
   LIENS NON ORIENTÉS (FIX)
   =========================== */

function compareRouterIds(a, b) {
  // Si format "R<number>", on compare numériquement (R2 < R10)
  const ma = /^R(\d+)$/.exec(a);
  const mb = /^R(\d+)$/.exec(b);
  if (ma && mb) return Number(ma[1]) - Number(mb[1]);

  // Sinon fallback alphabétique
  return String(a).localeCompare(String(b), 'en');
}

function canonicalLink(from, to) {
  const a = String(from);
  const b = String(to);
  return compareRouterIds(a, b) <= 0 ? [a, b] : [b, a];
}

function canonicalLinkId(from, to) {
  const [a, b] = canonicalLink(from, to);
  return `${a}-${b}`;
}

/* ===========================
   UDP
   =========================== */

// Traitement des messages UDP
udpServer.on('message', (msg, rinfo) => {
  const timestamp = new Date().toISOString();
  let data;

  try {
    data = JSON.parse(msg.toString());
    console.log(`[${timestamp}] Message reçu de ${rinfo.address}:${rinfo.port}`);
    console.log('Contenu:', data);

    // Traiter selon le type de message
    switch (data.type) {
      case 'PING':
        handlePing(data, timestamp);
        break;
      case 'LINK':
        handleLink(data, timestamp);
        break;
      default:
        console.warn('Type de message inconnu:', data.type);
    }

    // Relayer le message brut au client Web
    io.emit('raw-message', {
      timestamp,
      source: `${rinfo.address}:${rinfo.port}`,
      data
    });

  } catch (error) {
    console.error('Erreur parsing JSON:', error.message);
    io.emit('error', {
      timestamp,
      message: 'JSON invalide reçu',
      raw: msg.toString()
    });
  }
});

// Gestion des annonces PING
function handlePing(data, timestamp) {
  const { id, ip } = data;

  if (!id) {
    console.error('Message PING sans ID');
    return;
  }

  const node = {
    id,
    ip: ip || 'unknown',
    lastSeen: timestamp
  };

  const isNew = !networkState.nodes.has(id);
  networkState.nodes.set(id, node);

  // Notifier le client Web
  io.emit('node-update', {
    node,
    isNew
  });

  console.log(`${isNew ? '+ Nouveau' : '↻ Mise à jour'} routeur: ${id}`);
}

// Gestion des annonces LINK (non orienté)
function handleLink(data, timestamp) {
  const { from, to, status } = data;

  if (!from || !to || !status) {
    console.error('Message LINK incomplet');
    return;
  }

  const [a, b] = canonicalLink(from, to);
  const linkId = `${a}-${b}`;

  const link = {
    id: linkId,
    from: a,
    to: b,
    status,
    lastUpdate: timestamp
  };

  const isNew = !networkState.links.has(linkId);

  if (status === 'UP') {
    networkState.links.set(linkId, link);
    io.emit('link-update', { link, isNew });
    console.log(`${isNew ? '+ Nouveau' : '↻ Mise à jour'} lien: ${a} ↔ ${b} (${status})`);
  } else if (status === 'DOWN') {
    networkState.links.delete(linkId);
    // On envoie aussi l'id canonique pour que le front supprime sans ambiguïté
    io.emit('link-remove', { id: linkId, from: a, to: b });
    console.log(`✗ Lien supprimé: ${a} ↔ ${b} (DOWN)`);
  }
}

// Gestion des erreurs UDP
udpServer.on('error', (err) => {
  console.error('Erreur serveur UDP:', err);
  udpServer.close();
});

// Démarrage du serveur UDP
udpServer.bind(UDP_PORT, () => {
  console.log(`\n╔════════════════════════════════════════════╗`);
  console.log(`║   Gateway de Visualisation - Routage      ║`);
  console.log(`╠════════════════════════════════════════════╣`);
  console.log(`║  UDP Listener : localhost:${UDP_PORT}          ║`);
  console.log(`║  HTTP Server  : http://localhost:${HTTP_PORT}   ║`);
  console.log(`╚════════════════════════════════════════════╝\n`);
  console.log('En attente des messages des routeurs...\n');
});

// Démarrage du serveur HTTP
server.listen(HTTP_PORT, '0.0.0.0', () => {
  const networkInterfaces = require('os').networkInterfaces();
  let localIP = 'localhost';

  // Trouver l'IP locale (WiFi/Ethernet)
  Object.values(networkInterfaces).forEach(iface => {
    iface.forEach(config => {
      if (config.family === 'IPv4' && !config.internal) {
        localIP = config.address;
      }
    });
  });

  console.log(`\n📡 Dashboard accessible sur:`);
  console.log(`   Local:   http://localhost:${HTTP_PORT}`);
  console.log(`   Réseau:  http://${localIP}:${HTTP_PORT}`);
  console.log(`\nPartagez l'URL réseau avec vos appareils ! 🌐\n`);
});

// Nettoyage automatique des nœuds inactifs (optionnel, timeout 30s)
setInterval(() => {
  const now = new Date();
  const timeout = 30000; // 30 secondes

  networkState.nodes.forEach((node, id) => {
    const lastSeen = new Date(node.lastSeen);
    if (now - lastSeen > timeout) {
      console.log(`⚠ Timeout: Routeur ${id} inactif`);
      networkState.nodes.delete(id);

      // Supprimer les liens associés
      networkState.links.forEach((link, linkId) => {
        if (link.from === id || link.to === id) {
          networkState.links.delete(linkId);
          io.emit('link-remove', { id: linkId, from: link.from, to: link.to });
        }
      });

      io.emit('node-remove', { id });
    }
  });
}, 10000); // Vérification toutes les 10 secondes
