import { hoverLabel } from "./doughnut_plugins.js";
console.log(blosoftDB);

class ChartDataHelper {
  static getGoodUnit(value) {
    let max = 0;
    for (let i = 0; i < value.length; i++) {
      if (value[i] > max) {
        max = value[i];
      }
    }
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

  static getGoodValue(value, unit) {
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
      new_value[i] = Math.round(new_value[i] * 100) / 100;
    }
    return new_value;
  }

  static sortDataByMaxValue(categories, data) {
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

  static filterDataPercentages(categories, data, threshold) {
    let new_categories = [];
    let new_data = [];
    let percentage;
    let sum = 0;
    for (let i = 0; i < categories.length; i++) {
      sum += data[i];
    }
    for (let i = 0; i < categories.length; i++) {
      percentage = (data[i] * 100) / sum;
      if (percentage > threshold) {
        new_categories.push(categories[i]);
        new_data.push(data[i]);
      } else {
        break;
      }
    }
    return [new_categories, new_data];
  }
}

class RadarChart {
  constructor(labels, data, unit, vertData) {
    this.labels = labels;
    this.data = ChartDataHelper.getGoodValue(data, unit);
    this.vertData = ChartDataHelper.getGoodValue(vertData, unit);
    this.unit = unit;
  }

  generateConfig() {
    return {
      type: 'radar',
      data: {
        labels: this.labels,
        datasets: [
          {
            label: 'Your average data usage today (' + this.unit + ')',
            data: this.data,
            backgroundColor: 'rgba(200, 108, 24, 0.4)',
            borderColor: 'rgba(200, 108, 24, 1)',
          },
          {
            label: 'Vertuous usage (' + this.unit + ')',
            data: this.vertData,
            backgroundColor: 'rgba(144, 221, 140, 0.4)',
            borderColor: 'rgba(44, 122, 40, 1)',
          },
        ],
      },
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
  }
}

class DoughnutChart {
  constructor(labels, data, unit) {
    this.labels = labels;
    this.data = ChartDataHelper.getGoodValue(data, unit);
    this.unit = unit;
  }

  generateConfig() {
    return {
      type: 'doughnut',
      data: {
        labels: this.labels,
        datasets: [
          {
            label: 'Consumption (' + this.unit + ')',
            data: this.data,
            borderColor: 'rgb(0, 0, 0)',
            borderWidth: 1,
            hoverBorderWidth: 3,
            backgroundColor: this.labels.map((label, index) => {
              return index % 2 === 0 ? 'rgba(200, 108, 24, 0.8)' : 'rgba(44,  122, 40, 0.8)';
            }),
            hoverBackgroundColor: this.labels.map((label, index) => {
              return index % 2 === 0 ? 'rgba(200, 108, 24, 1)' : 'rgba(44,  102, 50, 1)';
            }),
          },
        ],
      },
      options: {
        maintainAspectRatio: false,
        responsive: true,
        plugins: {
          legend: {
            display: false,
          },
          title: {
            display: true,
            text: 'Your data usage repartition (' + this.unit + ')',
            color: '#23160d',
          },
        },
      },
      plugins: [hoverLabel],
    };
  }
}

// Récupération des données depuis la base de données
const protocolsFromDB = new Promise((resolve) => {
  blosoftDB.getProtocolsTotals().then((categoryTotals) => {
    const protocols = categoryTotals.map((categoryTotal) => categoryTotal.protocol);
    const data = categoryTotals.map((categoryTotal) => categoryTotal.total_data_quantity);
    resolve([protocols, data]);
  });
});

// Récupération des données depuis la base de données
const categoriesFromDB = new Promise((resolve) => {
  blosoftDB.getCategoryTotals().then((categoryTotals) => {
    const categories = categoryTotals.map((categoryTotal) => categoryTotal.category);
    const data = categoryTotals.map((categoryTotal) => categoryTotal.total_data_quantity);
    resolve([categories, data]);
  });
});

Promise.all([categoriesFromDB, protocolsFromDB]).then(([[labels, data], [labels2, data2]]) => {
  [labels, data] = ChartDataHelper.sortDataByMaxValue(labels, data);
  [labels2, data2] = ChartDataHelper.sortDataByMaxValue(labels2, data2);
  [labels, data] = ChartDataHelper.filterDataPercentages(labels, data, 0.3);
  [labels2, data2] = ChartDataHelper.filterDataPercentages(labels2, data2, 0.3);

  const radarChart = new RadarChart(labels, data, ChartDataHelper.getGoodUnit(data), data);
  const doughnutChart = new DoughnutChart(labels2, data2, ChartDataHelper.getGoodUnit(data2));

  const myRadar = new Chart(document.getElementById('radar'), radarChart.generateConfig());
  const myDoughnut = new Chart(document.getElementById('doughnut'), doughnutChart.generateConfig());
});
