

const labels_bar = [
    'Streaming',
    'Web',
    'Mail',
    'Cloud',
    'Web-market',
    'truc',
    'Other',
];

const data_radar = {
  	labels: labels_bar,
  	datasets: [
    	{
      		label: 'Consumption (Mb)',
      		data: [1, 10, 55, 18, 20, 30, 45],
     		backgroundColor: 'rgba(240, 108, 14, 0.4)',
      		borderColor: 'rgba(240, 108, 14, 1)',	
    	},
    	{
      	label: 'vertuosity (Mb)',
      	data: [10, 30, 50, 20, 2, 3, 4],
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
        		},
        	title: {
        	  display: true,
        	  text: 'Your Division vs Vertuosité'
        	}
      	},
    },
};
const data_doughnut = {
	labels: labels_bar,
	datasets: [
	  {
		label: 'Consumption (Mb)',
		data: [1, 10, 55, 18, 20, 30, 45],
		backgroundColor: 
			['rgba(240, 108, 14, 0.9)',
			'rgba(44, 122, 40, 0.9)',
			'rgba(240, 108, 14, 0.9)',
			'rgba(44, 122, 40, 0.9)',
			'rgba(240, 108, 14, 0.9)',
			'rgba(44, 122, 40, 0.9)',
			'rgba(240, 108, 14, 0.9)',
			],
		borderColor:'rgb(0, 0, 0)',
		hoverBorderWidth: 5,
		hoverBackgroundColor: 
			['rgba(200, 108, 24, 1)',
			'rgba(44, 102, 50, 1)',
			'rgba(200, 108, 24, 1)',
			'rgba(44, 102, 50, 1)',
			'rgba(200, 108, 24, 1)',
			'rgba(44, 102, 50, 1)',
			'rgba(200, 108, 24, 1)',
			],
	  	},
	],
};
//hoverLabel plugin 
//const hoverLabel = {
//	id: 'hoverLabel',
//	afterDraw(chart, args, options) {
//		const {ctx, chartArea: {left, right, top, bottom, width, height}} = chart;
//		ctx.save();
//		ctx.font = 'bolder 60px Arial';
//		ctx.fillStyle = 'blue';
//		ctx.fillText('Test', width / 2, height / 2);
//	},
//}

const config_doughnut = {
    type: 'doughnut',
    data: data_doughnut ,
    options: {
        maintainAspectRatio: false,
        responsive: true,
        plugins:{
			hoverLabel,
          	legend: {
				display: false,
        		},
        		title: {
        		  	display: true,
        		  	text: 'Your Division'
        	}
      	},
    },
};


const myRadar = new Chart(
  	document.getElementById('radar'),
 	config_radar
);

const myDoughnut = new Chart(
	document.getElementById('doughnut'),
	config_doughnut
);