const labels1 = [
    'August',
    'September',
    'October',
    'November',
    'December',
    'January',
];

const data1 = {
    labels: labels1,
    datasets: [{
        label: 'Your consumption over the past months (Go)',
        backgroundColor: 'rgba(200, 108, 24, 1)',
        borderColor: 'rgba(200, 108, 24, 1)',
        data: [53, 179, 150, 133, 79, 140],
        cubicInterpolationMode: 'monotone',
        fill: false,
    }]
};

const labels2 = [
    'Wednesday',
    'Thursday',
    'Friday',
    'Saturday',
    'Sunday',
    'Monday',
    'Tuesday',
];

const data2 = {
    labels: labels2,
    datasets: [{
        label: 'Your consumption over the past days (Go)',
        backgroundColor: 'rgba(200, 108, 24, 1)',
        borderColor: 'rgba(200, 108, 24, 1)',
        data: [4.2, 1.5, 7.8, 3.2, 9.1, 5.7, 4.5],
        cubicInterpolationMode: 'monotone',
        fill: false,
    }]
};

const options = {
    plugins: {
        legend: {
            labels: {
                color: '#23160d'
            }
        }
    },
    scales: {
        x: {
            ticks: {
                color: '#23160d',
            }
        },
        y: {
            ticks: {
                color: '#23160d',
            }
        }
    }
};

const config1 = {
    type: 'line',
    data: data1,
    options: options
};

const myChart = new Chart(
    document.getElementById('myChart'),
    config1,
);

const config2 = {
    type: 'line',
    data: data2,
    options: options
};

const myChart2 = new Chart(
    document.getElementById('myChart2'),
    config2
);

const DATA_COUNT = 7;
const NUMBER_CFG = {count: DATA_COUNT, min: 0, max: 100};

const labels_bar = [
    'Streaming',
    'Web browsing',
    'Email & IM',
    'Cloud storage',
    'Web-market',
    'Other',
];

const data_bar = {
  	labels: labels_bar,
  	datasets: [
    	{
      	label: 'Your data usage today (Go)',
      	data: [1.5, 0.47, 0.28, 0.56, 0.99, 0.7],
      	backgroundColor: 'rgba(44,  122, 40, 0.8)',
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
                labels: {
                    color: '#23160d'
                }  
        },
        title: {
          	display: true,
          	text: 'Your data usage today (Go)',
            color: '#23160d'
        }
      },
      scales: {
        x: {
            ticks: {
                color: '#23160d',
            }
        },
        y: {
            ticks: {
                color: '#23160d',
            }
        }
    }
    },
};
const myChart3 = new Chart(
    document.getElementById('myChart3'),
    config_bar
);