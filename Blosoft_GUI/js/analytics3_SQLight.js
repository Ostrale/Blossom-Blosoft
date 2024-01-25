// Fonction pour convertir une date en timestamp
function dateToTimestamp(date) {
    return Math.floor(date.getTime() / 1000);
}

// Fonction pour convertir un timestamp en date
function timestampToDate(timestamp) {
    return new Date(timestamp * 1000);
}

function getTotalsBetweenTimestamps() {
    // Calculate start and end timestamps for the week
    let now = new Date();
    let start_timestamp = dateToTimestamp(now) - 7 * 24 * 60 * 60;
    return new Promise((resolve, reject) => {
        blosoftDB.getTotalsBetweenTimestamps(start_timestamp).then((data) => {
            resolve(data[0].total_data_quantity);
        }).catch((error) => {
            reject(error);
        });
    });
}


document.addEventListener('DOMContentLoaded', function() {
    // CO2 equivalents (g per km)
    const trainEquivalence      = 2.3;
    const subwayEquivalence     = 4.2;
    const busEquivalence        = 104;
    const tshirtEquivalence     = 5149.616900739109; // g per tshirt made
    const motorcycleEquivalence = 165;
    const carEquivalence        = 96;
    const fridgeEquivalence     = 8832.40334/12; // g per month of use
    const coffeeEquivalence     = 28048.7772/12; // g per month of use

    // Simplified 1-byte model constants
    const Kdatacenter = 7.2 * 10**(-11); // kWh per byte
    const Kwifi = 1.52 * 10**(-10); // kWh per byte
    const Ffrance = 59; // g CO2 per kWh

    getTotalsBetweenTimestamps().then((data) => {
        const conso = data

        //const consoCO2 = conso * (Kdatacenter + Kwifi) * Ffrance; // g CO2

        const trainCO2 = conso * (Kdatacenter + Kwifi) * Ffrance / trainEquivalence;
        const subwayCO2 = conso * (Kdatacenter + Kwifi) * Ffrance / subwayEquivalence;
        const busCO2 = conso * (Kdatacenter + Kwifi) * Ffrance / busEquivalence;
        const tshirtCO2 = conso * (Kdatacenter + Kwifi) * Ffrance / tshirtEquivalence;
        const motorcycleCO2 = conso * (Kdatacenter + Kwifi) * Ffrance / motorcycleEquivalence;
        const carCO2 = conso * (Kdatacenter + Kwifi) * Ffrance / carEquivalence;
        const fridgeCO2 = conso * (Kdatacenter + Kwifi) * Ffrance / fridgeEquivalence;
        const coffeeCO2 = conso * (Kdatacenter + Kwifi) * Ffrance / coffeeEquivalence;
    
        // Update HTML elements
        document.getElementById("car").innerHTML = carCO2.toFixed(2);
        document.getElementById("subway").innerHTML = subwayCO2.toFixed(2);
        document.getElementById("train").innerHTML = trainCO2.toFixed(2);
        document.getElementById("fridge").innerHTML = fridgeCO2.toFixed(2);
        document.getElementById("motorcycle").innerHTML = motorcycleCO2.toFixed(2);
        document.getElementById("bus").innerHTML = busCO2.toFixed(2);
        document.getElementById("tshirt").innerHTML = tshirtCO2.toFixed(2);
        document.getElementById("coffee").innerHTML = coffeeCO2.toFixed(2);
    }).catch((error) => {
        console.error(error);
    });
});