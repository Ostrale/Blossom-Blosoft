const labels = [
    'January',
    'February',
    'March',
    'April',
    'May',
    'June',
];

const data = {
    labels: labels,
    datasets: [{
        label: 'My First dataset',
        backgroundColor: 'rgb(50, 50, 150)',
        borderColor: 'rgb(50, 50, 150)',
        data: [0, 10, 5, 2, 20, 30, 45],
        cubicInterpolationMode: 'monotone',
        fill: false,
    }]
};

const config = {
    type: 'line',
    data: data,
    options: {}
};

const myChart = new Chart(
    document.getElementById('myChart'),
    config
);

const myChart2 = new Chart(
    document.getElementById('myChart2'),
    config
);

const DATA_COUNT = 7;
const NUMBER_CFG = {count: DATA_COUNT, min: 0, max: 100};

const labels_bar = [
    'Streaming',
    'Web',
    'Mail',
    'Cloud',
    'Web-market',
    'Other',
];

const data_bar = {
  	labels: labels_bar,
  	datasets: [
    	{
      	label: 'Consumption (Mb)',
      	data: [1, 10, 5, 2, 20, 30, 45],
      	backgroundColor: 'rgb(50, 50, 150)',
    	}
  	]
};
const config_bar = {
    type: 'bar',
    data: data_bar,
    options: {
        maintainAspectRatio: false,
        responsive: true,
        plugins: {
          	legend: {
          	position: 'left',
        },
        title: {
          	display: true,
          	text: 'Your Division'
        }
      }
    },
};
const myChart3 = new Chart(
    document.getElementById('myChart3'),
    config_bar
);