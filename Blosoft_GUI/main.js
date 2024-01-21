// main.js

// importation des modules
const databaseFunctions = require('./js_backend/databaseFunctions.js');
const utiles = require('./js_backend/utiles.js');


// Modules de controle du cycle de vie de l'application et de création 
// de fenêtre native de navigateur
const { app, BrowserWindow, globalShortcut, ipcMain  } = require('electron')
const path = require('path')

var devtoolsstate = false;

const createWindow = () => {
  	// Création de la fenêtre de navigateur.
  	const mainWindow = new BrowserWindow({
		minWidth: 510,
    	width: 800,
    	height: 600,
    	webPreferences: {
      		preload: path.join(__dirname, 'preload.js')
    	}
	})
	//on ne veut pas de la bare de menu
	mainWindow.removeMenu()
  	// et chargement de l'index.html de l'application.
  	mainWindow.loadFile('index.html')

  	// Ouvrir les outils de développement.
  	//mainWindow.webContents.openDevTools()

	//F5 actualiser page
	globalShortcut.register('f5', function() {
		mainWindow.reload()
	});
	//F9 Ouvrir les outils de développement
	globalShortcut.register('f9', function() {
		if(!devtoolsstate){
			mainWindow.webContents.openDevTools();
			devtoolsstate = true;
		}else{
			mainWindow.webContents.closeDevTools();
			devtoolsstate = false;
		}	
	});
}	

// Cette méthode sera appelée quand Electron aura fini
// de s'initialiser et sera prêt à créer des fenêtres de navigation.
// Certaines APIs peuvent être utilisées uniquement quant cet événement est émit.
app.whenReady().then(() => {
	// --------------------- Section pour les IPC -----------------------------------------------------------
	// Ne pas oubliez d'intégrer aussi dans le fichier preload.js

	ipcMain.on('log', utiles.log); // utilisation dans un js de page web: electronAPI.log('message');
	ipcMain.handle('get_categories', databaseFunctions.getCategories); // utilisation dans un js de page web: blosoftDB.getCategories()
	ipcMain.handle('get_category_totals', databaseFunctions.getCategoryTotals);
	ipcMain.handle('get_category_totals_between_timestamps', databaseFunctions.getCategoryTotalsBetweenTimestamps);
	// --------------------- Fin section pour les IPC -------------------------------------------------------

	// Création de la fenêtre de navigateur.
  	createWindow()
  	app.on('activate', () => {
    	// Sur macOS il est commun de re-créer une fenêtre  lors 
    	// du click sur l'icone du dock et qu'il n'y a pas d'autre fenêtre ouverte.
    	if (BrowserWindow.getAllWindows().length === 0) createWindow()
  })
})

// Quitter quand toutes les fenêtres sont fermées, sauf sur macOS. Dans ce cas il est courant
// que les applications et barre de menu restents actives jusqu'à ce que l'utilisateur quitte 
// de manière explicite par Cmd + Q.
app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit()
})

// Dans ce fichier vous pouvez inclure le reste du code spécifique au processus principal. Vous pouvez également le mettre dans des fichiers séparés et les inclure ici.
