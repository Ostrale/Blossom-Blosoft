import { hoverLabel } from "./doughnut_plugins.js";

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

function sort_by_max_value(categories, data) {
	// prend une liste de catégories et une liste de valeurs et renvoie les listes triées par ordre décroissant de valeurs
	let new_categories = [];
	let new_data = [];
	let max = 0;
	let index = 0;
	for (let i = 0; i < categories.length; i++) {
		for (let j = 0; j < categories.length; j++) {
			if (data[j] > max) {
				max = data[j];
				index = j;
			}
		}
		new_categories.push(categories[index]);
		new_data.push(data[index]);
		data[index] = 0;
		max = 0;
	}
	return [new_categories, new_data];
}


// Remplissage de labels_bar avec les données de la base de données
const labels_bar = new Promise((resolve) => {
	blosoftDB.getCategories().then((categories) => {
		const labels = categories.map((category) => category.categories);
		resolve(labels);
	});
});

// Récupération des données depuis la base de données
const data_from_db = new Promise((resolve) => {
	blosoftDB.getCategoryTotals().then((categoryTotals) => {
		const data = categoryTotals.map((categoryTotal) => categoryTotal.total_quantity);
		resolve(data);
	});
});

Promise.all([labels_bar, data_from_db]).then(([labels, data]) => {

	// sort the data
	[labels, data] = sort_by_max_value(labels, data);

	let vert = [];
	for (let i = 0; i < labels.length; i++) {
		if (i%2 == 0){
			vert.push(data[i] / 1.5);
		} else {
			vert.push(data[i] * 1.5);
		}
		
	}

	// get the good unit
	const unit = get_good_unit(data);
	// get the good value
	data = get_good_value(data, unit);
	vert = get_good_value(vert, unit);
	

	const data_radar = {
		labels: labels,
		datasets: [
			{
				label: 'Your average data usage today (' + unit + ')',
				data: data,
				backgroundColor: 'rgba(200, 108, 24, 0.4)',
				borderColor: 'rgba(200, 108, 24, 1)',
			},
			{
				label: 'Vertuous usage (' + unit + ')',
				data: vert,
				backgroundColor: 'rgba(144, 221, 140, 0.4)',
				borderColor: 'rgba(44, 122, 40, 1)',
			},
		],
	};

	const config_radar = {
		type: 'radar',
		data: data_radar,
		options: {
			maintainAspectRatio: false,
			responsive: true,
			plugins: {
				legend: {
					position: 'bottom',
					labels: {
						color: '#23160d',
					},
				},
				title: {
					display: true,
					text: 'Your average data consumption vs Vertuous average data consumption',
					color: '#23160d',
				},
			},
			scales: {
				r: {
					angleLines: {
						color: '#999999',
					},
					grid: {
						color: '#999999',
					},
					pointLabels: {
						color: '#23160d',
					},
					ticks: {
						color: '#23160d',
					},
				},
			},
		},
	};

	const data_doughnut = {
		labels: labels,
		unit_hoverLabel: unit,
		datasets: [
		  {
			label: 'Consumption (' + unit + ')',
			data: data,
			borderColor:'rgb(0, 0, 0)',
			borderWidth: 1,
			hoverBorderWidth: 3,
			backgroundColor: labels.map((label, index) => {
                return index % 2 === 0 ? 'rgba(200, 108, 24, 0.8)' : 'rgba(44,  122, 40, 0.8)';
            }),
            hoverBackgroundColor: labels.map((label, index) => {
                return index % 2 === 0 ? 'rgba(200, 108, 24, 1)' : 'rgba(44,  102, 50, 1)';
            }),
			  },
		],
	};
	
	//hoverLabel plugin 
	const config_doughnut = {
		type: 'doughnut',
		data: data_doughnut ,
		options: {
			maintainAspectRatio: false,
			responsive: true,
			plugins:{
				  legend: {
					display: false,
					},
					title: {
						  display: true,
						  text: 'Your data usage repartition (' + unit + ')',
						color: '#23160d'
				}
			  },
		},
		plugins: [hoverLabel]
	};
	
	
	const myRadar = new Chart(
		  document.getElementById('radar'),
		 config_radar
	);
	
	const myDoughnut = new Chart(
		document.getElementById('doughnut'),
		config_doughnut
	);
});



