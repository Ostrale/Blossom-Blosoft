// Fonction pour convertir une date en timestamp
function dateToTimestamp(date) {
    return Math.floor(date.getTime() / 1000);
}

// Fonction pour convertir un timestamp en date
function timestampToDate(timestamp) {
    return new Date(timestamp * 1000);
}

function getRandomInt(max) {
    return Math.floor(Math.random() * max);
}

function get_good_unit(value) {
	// prend une liste de valeurs en octets et renvoie une unité adaptée à la plus grande des valeurs de la liste

	// find the biggest value
	let max = 0;
	for (let i = 0; i < value.length; i++) {
		if (value[i] > max) {
			max = value[i];
		}
	}
	// find the unit
	let unit = 'o';
	if (max > 1024) {
		unit = 'Ko';
	}
	if (max > 1024 * 1024) {
		unit = 'Mo';
	}
	if (max > 1024 * 1024 * 1024) {
		unit = 'Go';
	}
	if (max > 1024 * 1024 * 1024 * 1024) {
		unit = 'To';
	}

	return unit;
}

function get_good_value(value, unit) {
	// prend une liste de valeurs en octets et renvoie une liste de valeurs adaptées à l'unité
	let new_value = [];
	for (let i = 0; i < value.length; i++) {
		if (unit == 'Ko') {
			new_value.push(value[i] / 1024);
		}
		if (unit == 'Mo') {
			new_value.push(value[i] / 1024 / 1024);
		}
		if (unit == 'Go') {
			new_value.push(value[i] / 1024 / 1024 / 1024);
		}
		if (unit == 'To') {
			new_value.push(value[i] / 1024 / 1024 / 1024 / 1024);
		}
		if (unit == 'o') {
			new_value.push(value[i]);
		}
		// round to 2 decimals
		new_value[i] = Math.round(new_value[i] * 100) / 100;
	}
	return new_value;
}


function getTotalsEcoscore() {
    const MAX_CONSO = 16439574528; // 1570 mb
    const LOOSE_POINT = 16439574528 * 0.1;
    let score = 100;
    let now = new Date();
    let start_timestamp = dateToTimestamp(now) - 7 * 24 * 60 * 60;
    return new Promise((resolve, reject) => {
        blosoftDB.getTotalsBetweenTimestamps(start_timestamp).then((data) => {
            data = data[0].total_data_quantity;
            if (data > MAX_CONSO) {
                let depacement = data - MAX_CONSO;
                let nb_point_perdu = (depacement / LOOSE_POINT)*10;
                score = score - nb_point_perdu;
            }
            resolve([score, data]);
        }).catch((error) => {
            reject(error);
        });
    });
}


//var pourcentage = getRandomInt(101);
getTotalsEcoscore().then(([pourcentage, data]) => {
    
    var pourcentage = pourcentage.toFixed(0);
    if (pourcentage < 10){
        var Pourcentage_entier = 0;
    }else if (pourcentage == 100){
        var Pourcentage_entier = 9;
    }else{
        var Pourcentage_texte = pourcentage.toString();
        var Pourcentage_texte = Pourcentage_texte.slice(0, 1);
        var Pourcentage_entier = parseInt(Pourcentage_texte);
    }
    
    let var3 = "<img src='image/arbre_" + Pourcentage_entier + ".png 'alt='Photo dun arbre'/>";
    console.log(var3);
    let imageinp = document.getElementById("arbre");
    imageinp.innerHTML = var3;
    
    let ecoscore = document.getElementById("ecoscore");
    ecoscore.innerHTML = "Your Ecoscore : "+pourcentage;

    unit = get_good_unit([data]);
    data = get_good_value([data], unit);
    let litle_text = document.getElementById("text");
    litle_text.innerHTML = "You have consumed "+data+" "+ unit+" bytes last 7 days";


}).catch((error) => {
    console.error(error);
});

