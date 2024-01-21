// preload.js

const { contextBridge, ipcRenderer } = require('electron')

// Toutes les API Node.js sont disponibles dans le processus de préchargement.
// Il a la même sandbox qu'une extension Chrome.
window.addEventListener('DOMContentLoaded', () => {
    const replaceText = (selector, text) => {
      const element = document.getElementById(selector)
      if (element) element.innerText = text
    }
  
    for (const dependency of ['chrome', 'node', 'electron']) {
      replaceText(`${dependency}-version`, process.versions[dependency])
    }
  })

// Exposez des méthodes qui doivent être accessibles à partir du processus de rendu.
// Toutes les méthodes exposées seront disponibles sur l'objet `window.electron`.
contextBridge.exposeInMainWorld('electronAPI', {
  log: (message) => {
    ipcRenderer.send('log', message); // utilisation dans un js de page web: electronAPI.log('message');
  }
})

// Expose custom API to the window object
contextBridge.exposeInMainWorld('blosoftDB', {
  getCategories: () => ipcRenderer.invoke('get_categories'),
  getCategoryTotals: () => ipcRenderer.invoke('get_category_totals'),
  getCategoryTotalsBetweenTimestamps: (startTimestamp, endTimestamp) => ipcRenderer.invoke('get_category_totals_between_timestamps', startTimestamp, endTimestamp),
});
