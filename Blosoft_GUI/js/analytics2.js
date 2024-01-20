import { hoverLabel } from "./doughnut_plugins.js";

const labels_bar = [
    'Web browsing',
	'Streaming',
    'Email',
    'Social media',
    'Web-market',
    'Other',
];

const data_radar = {
  	labels: labels_bar,
  	datasets: [
    	{
      		label: 'Your average data usage today (Go)',
      		data:[4.45, 8.05, 3.45, 9.35, 5.95, 6.75],
     		backgroundColor: 'rgba(200, 108, 24, 0.4)',
      		borderColor: 'rgba(200, 108, 24, 1)',	
    	},
    	{
      	label: 'Vertuous usage (Go)',
      	data: [0.810, 9.54, 0.520, 3.535, 1.02, 1.055], 
      	backgroundColor: 'rgba(144, 221, 140, 0.4)',
      	borderColor: 'rgba(44, 122, 40, 1)',
    	}
  	]
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
				color: '#23160d'
			}  
        		},
        	title: {
        	  display: true,
        	  text: 'Your average data consumption vs Vertuous average data consumption',
			  color: '#23160d'
        	}
      	},
	scales: {
      r: {
		angleLines: {
			color: '#999999'
		  },
        grid: {
          color: '#999999'
        },
		pointLabels: {
			color: '#23160d'
		  },
		ticks: {
			color: '#23160d'
		  },
      }
    }
    },
};

const data_doughnut = {
	labels: labels_bar,
	datasets: [
	  {
		label: 'Consumption (Go)',
		data: [0.470, 8.500, 0.280, 2.560, 0.990, 0.700],
		backgroundColor: 
			['rgba(200, 108, 24, 0.8)',
			 'rgba(44,  122, 40, 0.8)',
			 'rgba(200, 108, 24, 0.8)',
			 'rgba(44,  122, 40, 0.8)',
			 'rgba(200, 108, 24, 0.8)',
			 'rgba(44,  122, 40, 0.8)',
			],
		borderColor:'rgb(0, 0, 0)',
		borderWidth: 1,
		hoverBorderWidth: 3,
		hoverBackgroundColor: 
			['rgba(200, 108, 24, 1)',
			 'rgba(44,  102, 50, 1)',
			 'rgba(200, 108, 24, 1)',
			 'rgba(44,  102, 50, 1)',
			 'rgba(200, 108, 24, 1)',
			 'rgba(44,  102, 50, 1)',
			],
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
        		  	text: 'Your data usage repartition (Go)',
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