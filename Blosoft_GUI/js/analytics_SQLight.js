// Fonction pour convertir une date en timestamp
function dateToTimestamp(date) {
    return Math.floor(date.getTime() / 1000);
}

// Fonction pour convertir un timestamp en date
function timestampToDate(timestamp) {
    return new Date(timestamp * 1000);
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

// Fonction pour convertir un temps en millisecondes
function convertToMilliseconds(timeValue, unit) {
    switch (unit) {
        case 'millisecond':
            return timeValue;
        case 'second':
            return timeValue * 1000;
        case 'minute':
            return timeValue * 60 * 1000;
        case 'hour':
            return timeValue * 60 * 60 * 1000;
        case 'day':
            return timeValue * 24 * 60 * 60 * 1000;
        case 'week':
            return timeValue * 7 * 24 * 60 * 60 * 1000;
        case 'month':
            return timeValue * 30 * 24 * 60 * 60 * 1000; // Approximation
        case 'year':
            return timeValue * 365 * 24 * 60 * 60 * 1000; // Approximation
        default:
            throw new Error(`Unknown time unit: ${unit}`);
    }
}

// Fonction pour convertir un temps en secondes
function convertToSeconds(timeValue, unit) {
    return convertToMilliseconds(timeValue, unit) / 1000;
}

class DataGraph_line {
    constructor(chartElementId, unit, numberOfIntervals, timeStep = 1, delaySeconds = 60) {
        this.chartElementId = chartElementId;
        this.unit = unit;
        this.numberOfIntervals = numberOfIntervals;
        this.timeStep = timeStep;
        this.delaySeconds = delaySeconds;

        this.unit_data = 'x';

        this.myChart = null;
        this.initializeChart();
        this.updateData('linear');
        setInterval(() => {
            this.updateData();
        },convertToMilliseconds(this.timeStep*0.5, this.unit));
    }

    generateTimestamps() {
        let now = new Date();
        now = new Date(now.getTime() - convertToMilliseconds(this.delaySeconds, 'second'));
        let now_timestamp = dateToTimestamp(now) - this.delaySeconds ;

        // faire un case pour chaque unité de temps et enlever l'unité de temps du dessous 
        // (ex: si on veut des intervalles de 1h, on enlève les secondes, minutes et millisecondes)
        // (ex: si on veut des intervalles de 1j, on enlève les secondes, minutes, millisecondes et heures)
        // (ex: si on veut des intervalles de 1min, on enlève les secondes et millisecondes)
        let seconds = now.getSeconds();
        let minutes = now.getMinutes();
        let hours = now.getHours();
        let days = now.getDate();
        switch (this.unit) {

            case 'second':
                break;
            case 'minute':
                if (seconds !== 0) {
                    now_timestamp -= seconds;
                }
                break;
            case 'hour':
                if (seconds !== 0) {
                    now_timestamp -= seconds;
                }
                if (minutes !== 0) {
                    now_timestamp -= convertToSeconds(minutes, 'minute');
                }
                break;
            case 'day':
                if (seconds !== 0) {
                    now_timestamp -= seconds;
                }
                if (minutes !== 0) {
                    now_timestamp -= convertToSeconds(minutes, 'minute');
                }
                if (hours !== 0) {
                    now_timestamp -= convertToSeconds(hours, 'hour');
                }
                break;
            case 'week':
                if (seconds !== 0) {
                    now_timestamp -= seconds;
                }
                if (minutes !== 0) {
                    now_timestamp -= convertToSeconds(minutes, 'minute');
                }
                if (hours !== 0) {
                    now_timestamp -= convertToSeconds(hours, 'hour');
                }
                if (days !== 0) {
                    now_timestamp -= convertToSeconds(days, 'day');
                }
                break;
            default:
                break;
        }


        for (let i = 0; i < this.numberOfIntervals - 1; i++) {
            now_timestamp -= convertToSeconds(this.timeStep, this.unit);
        }

        let now_timestamp_2 = now_timestamp + convertToSeconds(this.timeStep, this.unit);
        let list_timestamp = [];

        for (let i = 0; i < this.numberOfIntervals; i++) {
            list_timestamp.push([now_timestamp, now_timestamp_2]);
            now_timestamp += convertToSeconds(this.timeStep, this.unit);
            now_timestamp_2 += convertToSeconds(this.timeStep, this.unit);
        }
        return list_timestamp;
    }

    timestamp_to_hour(timestamps) {
        let time;
        let list_hour = [];
        for (let i = 0; i < timestamps.length; i++) {
            time = timestampToDate(timestamps[i]).toLocaleTimeString();
            if (this.unit != 'second'){
                time = time.slice(0, -3);
            }
            list_hour.push(time);
        }
        return list_hour;
    }

    from_DB_get_last_x_y_data() {
        return new Promise((resolve, reject) => {
            let list_timestamp = this.generateTimestamps();
            var total_data = new Map();
            for (let i = 0; i < list_timestamp.length; i++) {
                blosoftDB.getTotalsBetweenTimestamps(list_timestamp[i][0], list_timestamp[i][1]).then((result) => {
                    const data_quantity = result[0].total_data_quantity;
                    total_data.set(list_timestamp[i][1], data_quantity);
                    if (total_data.size === list_timestamp.length) {
                        total_data = new Map([...total_data.entries()].sort());
                        resolve(total_data);
                    }
                }).catch((error) => {
                    reject(error);
                });
            }
        });
    }

    initializeChart() {
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
        this.myChart = new Chart(
            document.getElementById(this.chartElementId),
            {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: `Your consumption over the past ${this.numberOfIntervals * this.timeStep} ${this.unit}s in ${this.unit_data}`,
                        backgroundColor: 'rgba(200, 108, 24, 1)',
                        borderColor: 'rgba(200, 108, 24, 1)',
                        data: [],
                        cubicInterpolationMode: 'monotone',
                        fill: false,
                    }]
                },
                options: options
            }
        );
    }

    updateData(animation = 'none') {
        this.from_DB_get_last_x_y_data().then((total_data) => {
            const labels1 = this.timestamp_to_hour([...total_data.keys()]);
            let list_total_data = [...total_data.values()];
    
            this.unit_data = get_good_unit(list_total_data);
            list_total_data = get_good_value(list_total_data, this.unit_data);

            // Mise à jour du titre dans datasets  label
            this.myChart.data.datasets[0].label = `Your consumption over the past ${this.numberOfIntervals * this.timeStep} ${this.unit}s in ${this.unit_data}`;

            // Ajout de nouvelles données
            this.myChart.data.labels.push(...labels1.slice(-this.numberOfIntervals));
            this.myChart.data.datasets[0].data.push(...list_total_data.slice(-this.numberOfIntervals));
    
            // Suppression des anciennes données si nécessaire
            if (this.myChart.data.labels.length > this.numberOfIntervals) {
                this.myChart.data.labels.splice(0, this.myChart.data.labels.length - this.numberOfIntervals);
                this.myChart.data.datasets[0].data.splice(0, this.myChart.data.datasets[0].data.length - this.numberOfIntervals);
            }
    
            // Mise à jour du graphique
            this.myChart.update(animation);
        }).catch((error) => {
            console.error("Error updating data:", error);
        });
    }
    
}

class DataGraph_bar {
    constructor(elementId, timeInterval, threshold, get_function = blosoftDB.getCategoryTotalsBetweenTimestamps) {
        this.elementId = elementId;
        this.threshold = threshold;
        this.timeInterval = timeInterval;
        this.get_function = get_function;
        this.unit = 'x';
    }

    sort_by_max_value(categories, data) {
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

    filter_percentages(categories, data) {
        let new_categories = [];
        let new_data = [];
        let percentage;
        let sum = 0;
        for (let i = 0; i < categories.length; i++) {
            sum += data[i];
        }
        for (let i = 0; i < categories.length; i++) {
            percentage = (data[i] * 100) / sum;
            if (percentage > this.threshold) {
                new_categories.push(categories[i]);
                new_data.push(data[i]);
            } else {
                break;
            }
        }
        return [new_categories, new_data];
    }

    get_time_interval() {
        let now = new Date();
        let now_timestamp = dateToTimestamp(now);
        let start_timestamp, end_timestamp;

        switch (this.timeInterval) {
            case 'hour':
                start_timestamp = now_timestamp - now.getMinutes() * 60 - now.getSeconds();
                end_timestamp = start_timestamp + 3600 - 1;
                break;
            case 'day':
                start_timestamp = now_timestamp - now.getHours() * 3600 - now.getMinutes() * 60 - now.getSeconds();
                end_timestamp = start_timestamp + 86400 - 1;
                break;
            case 'week':
                // Calculate start and end timestamps for the week
                start_timestamp = now_timestamp - now.getDay() * 86400 - now.getHours() * 3600 - now.getMinutes() * 60 - now.getSeconds();
                end_timestamp = start_timestamp + 7 * 86400 - 1;
                break;
            case 'month':
                // Calculate start and end timestamps for the month
                start_timestamp = now_timestamp - (now.getDate() - 1) * 86400 - now.getHours() * 3600 - now.getMinutes() * 60 - now.getSeconds();
                end_timestamp = start_timestamp + 30 * 86400 - 1;
                break;
            case 'year':
                // Calculate start and end timestamps for the year
                start_timestamp = now_timestamp - (now.getMonth() - 1) * 30 * 86400 - (now.getDate() - 1) * 86400 - now.getHours() * 3600 - now.getMinutes() * 60 - now.getSeconds();
                end_timestamp = start_timestamp + 365 * 86400 - 1;
                break;
            default:
                // Default to the day if an invalid interval is provided
                start_timestamp = now_timestamp - now.getHours() * 3600 - now.getMinutes() * 60 - now.getSeconds();
                end_timestamp = start_timestamp + 86400 - 1;
        }

        return [start_timestamp, end_timestamp];
    }

    async initialize() {
        const time_interval = this.get_time_interval();
        const conso_today = await  this.get_function(time_interval[0], time_interval[1]);
        const { labels, data } = this.processChartData(conso_today);

        let labels_bar = labels;
        let data_bar = {
            labels: labels_bar,
            datasets: [
                {
                    label: 'Your data usage this '+ this.timeInterval + ' in ' + this.unit,
                    data: data,
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
                        text: 'Your data usage this '+ this.timeInterval + ' in ' + this.unit,
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
            document.getElementById(this.elementId),
            config_bar
        );
    }

    processChartData(conso_today) {
        const labels = [];
        const data = [];

        const label_find = Object.keys(conso_today[0])[0];

        for (let i = 0; i < conso_today.length; i++) {
            labels.push(conso_today[i][label_find]);
            data.push(conso_today[i].total_data_quantity);
        }

        
        // sort the data
        const [sortedLabels, sortedData] = this.sort_by_max_value(labels, data);
        // filter the data
        const [filteredLabels, filteredData] = this.filter_percentages(sortedLabels, sortedData);
        // get the good unit
        const unit = get_good_unit(filteredData);
        this.unit = unit;
        // get the good value
        const processedData = get_good_value(filteredData, unit);
    
        return { labels: filteredLabels, data: processedData };
    }
    
}

class NumberValueChanger {
    constructor(selector_number, selector_title, usages = ['day', 'week'], compared_to = ['yesterday', 'last week']) {
        this.elements = document.querySelectorAll(selector_number);
        this.title = document.querySelectorAll(selector_title);

        let usage1 = usages[0];
        let usage2 = usages[1];
        let Compared1 = compared_to[0];
        let Compared2 = compared_to[1];

        let title = [
            'Your Ecoscore',
            "This " + usage1 + "'s Usage",
            'Compared to ' + Compared1,
            "This " + usage2 + "'s Usage",
            "Compared to " + Compared2,
            'Weeks streak'
        ]

        this.changeTitle(title);

        Promise.all([
            this.Your_Ecoscore(),
            this.Usage_Trend(usage1),
            this.Compared_to(Compared1),
            this.Usage_Trend(usage2),
            this.Compared_to(Compared2),
            this.Weeks_streak()
        ]).then((values) => {
            this.changeValues(values);
        });
    }

    get_consommation(t1 = -1, t2 = -1) {
        return new Promise((resolve, reject) => {
            if (t1 == -1 || t2 == -1) {
                t1 = 1;
                t2 = dateToTimestamp(new Date()) + 100; // now + 100 seconds for the delay (security)
            }
            blosoftDB.getTotalsBetweenTimestamps(t1, t2).then((result) => {
                const data_quantity = result[0].total_data_quantity;
                resolve(data_quantity);
            }).catch((error) => {
                reject(error);
            });
        });
    }

    async Your_Ecoscore() {
        const MAX_CONSO = 16439574528; // 1570 mb
        const LOOSE_POINT = 16439574528 * 0.1;
        let score = 100;
        const t1 = dateToTimestamp(new Date()) - 7 * 24 * 60 * 60; // 7 days ago
        const t2 = dateToTimestamp(new Date()) + 100; // now + 100 seconds for the delay (security)
        return new Promise((resolve, reject) => {
            this.get_consommation(t1, t2)
                .then((myscore) => {
                    if (myscore > MAX_CONSO) {
                        score -= ((myscore - MAX_CONSO) / LOOSE_POINT)*10;
                    }
                    resolve(score);
                })
                .catch((error) => {
                    console.error("Error updating data:", error);
                    reject(error);
                });
        });
    }

    async Usage_Trend(period) {
        let t1, t2;
        let now = new Date();

        switch (period) {
            case 'hour':
                t1 = dateToTimestamp(new Date()) - 60 * 60; // 1 hour ago
                break;
            case 'day':
                t1 = dateToTimestamp(new Date()) - now.getHours() * 3600 - now.getMinutes() * 60 - now.getSeconds();
                break;
            case 'week':
                t1 = dateToTimestamp(new Date()) - now.getDay() * 86400 + now.getHours() * 3600 + now.getMinutes() * 60 + now.getSeconds();
                break;
            case 'month':
                t1 = dateToTimestamp(new Date()) - (now.getDate() - 1) * 86400 - now.getHours() * 3600 - now.getMinutes() * 60 - now.getSeconds();
                break;
            case 'year':
                t1 = dateToTimestamp(new Date()) - (now.getMonth() - 1) * 30 * 86400 - (now.getDate() - 1) * 86400 - now.getHours() * 3600 - now.getMinutes() * 60 - now.getSeconds();
                break;
            default:
                throw new Error('Invalid period');
        }

        t2 = dateToTimestamp(new Date()) + 100; // now + 100 seconds for the delay (security)

        return new Promise((resolve, reject) => {
            this.get_consommation(t1, t2)
                .then((Usage) => {
                    // get good unit and value and return value + unit
                    const unit = get_good_unit([Usage]);
                    const value = get_good_value([Usage], unit);
                    resolve(value + ' ' + unit);
                })
                .catch((error) => {
                    console.error("Error updating data:", error);
                    reject(error);
                });
        });
    }

    async Compared_to(period) {
        let t0, t1, t2;
        let now = new Date();
        switch (period) {
            case 'last hour':
                t0 = dateToTimestamp(new Date()) + 100; // now + 100 seconds for the delay (security)
                t1 = dateToTimestamp(new Date()) - 60 * 60; // 1 hour ago
                t2 = dateToTimestamp(new Date()) - 2 * 60 * 60; // 2 hours ago
                break;
            case 'yesterday':
                t0 = dateToTimestamp(new Date()) + 100; // now + 100 seconds for the delay (security)
                t1 = t0 - now.getHours() * 3600 - now.getMinutes() * 60 - now.getSeconds();
                t2 = t1 - 24 * 60 * 60; // 48 hours ago
                break;
            case 'last week':
                t0 = dateToTimestamp(new Date()) + 100; // now + 100 seconds for the delay (security)
                t1 =  t0 - now.getDay() * 86400 + now.getHours() * 3600 + now.getMinutes() * 60 + now.getSeconds();
                t2 = t1 - 7 * 24 * 60 * 60; // 14 days ago
                break;
            default:
                throw new Error('Invalid period');
        }

         

        return new Promise((resolve, reject) => {
            this.get_consommation(t1, t0)
                .then((Usage) => {
                    this.get_consommation(t2, t1)
                        .then((Usage2) => {
                            // if Usage  is null set it to 0
                            if (!Usage) {
                                Usage = 0;
                            }
                            // get good unit and value and return value + unit
                            const diff = Usage - Usage2;
                            const percentage = (diff * 100) / Usage2;
                            let symbole = '';
                            if (percentage > 0) {
                                symbole = '+';
                            }
                            resolve(symbole + percentage.toFixed(1) + '%');
                        })
                        .catch((error) => {
                            console.error("Error updating data:", error);
                            reject(error);
                        });
                })
                .catch((error) => {
                    console.error("Error updating data:", error);
                    reject(error);
                });
        });
    }

    Weeks_streak() {
        return new Promise((resolve, reject) => {
            blosoftDB.getOldestTimestamp().then((result) => {
                let oldest_timestamp = result[0].oldest_timestamp;
                // convert timestamp to date and count the number of weeks between now and the oldest timestamp
                const now = new Date();
                const oldest_date = timestampToDate(oldest_timestamp);
                const diffTime = Math.abs(now - oldest_date);
                const diffWeeks = Math.ceil(diffTime / (1000 * 60 * 60 * 24 * 7));
                resolve(diffWeeks);
            }
            ).catch((error) => {
                console.error("Error updating data:", error);
                reject(error);
            });
        });
    }

    changeValues(args) {
        this.elements.forEach((element, index) => {
            element.textContent = args[index];
        });
    }

    changeTitle(args) {
        this.title.forEach((element, index) => {
            element.textContent = args[index];
        });
    }
    
}

  
console.log(blosoftDB);
// Exemple d'utilisation
const numberValueChanger = new NumberValueChanger('.number-value','.number-title', ['hour', 'day'], ['last hour', 'yesterday']);

// Crée une instance de la classe DataGraph_line en spécifiant l'ID de l'élément chart, l'intervalle de temps, l'unité de temps et le nombre d'intervalles
const DataGraph_line1 = new DataGraph_line('myChart', 'minute', 20, 3);
const DataGraph_line2 = new DataGraph_line('myChart2', 'second', 60);


const elementId = 'myChart3'; // Replace with desired element ID
const threshold = 0.3; // Replace with desired threshold
const timeInterval = 'hour'; // Replace with 'day', 'week', 'month', or 'year'
const functionToCall = blosoftDB.getProtocolsTotalsBetweenTimestamps; // default is blosoftDB.getCategoryTotalsBetweenTimestamps

const bar1 = new DataGraph_bar(elementId, timeInterval, threshold, functionToCall);
bar1.initialize();