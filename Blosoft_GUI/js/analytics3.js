// CO2 equivalents (kg per km)
const trainEquivalence      = 0.0023;
const subwayEquivalence     = 0.0042;
const busEquivalence        = 0.104;
const planeEquivalence      = 156.21; // kg per TLS-CDG flight
const motorcycleEquivalence = 0.165;
const carEquivalence        = 0.096;
const fridgeEquivalence     = 5.27273319; // kg per year of use
const boilerEquivalence     = 8.83240334; // kg per year of use

// Simplified 1-byte model constants
const Kdatacenter = 7.2 * 10**(-11); // kWh per byte
const Kwifi = 1.52 * 10**(-10); // kWh per byte
const Ffrance = 0.059; // kg CO2 per kWh

conso = 38 * 10**6; // 38Gb in bytes

const trainCO2 = conso * (Kdatacenter + Kwifi) * Ffrance * trainEquivalence;
const subwayCO2 = conso * (Kdatacenter + Kwifi) * Ffrance * subwayEquivalence;
const busCO2 = conso * (Kdatacenter + Kwifi) * Ffrance * busEquivalence;
const planeCO2 = conso * (Kdatacenter + Kwifi) * Ffrance * planeEquivalence;
const motorcycleCO2 = conso * (Kdatacenter + Kwifi) * Ffrance * motorcycleEquivalence;
const carCO2 = conso * (Kdatacenter + Kwifi) * Ffrance * carEquivalence;
const fridgeCO2 = conso * (Kdatacenter + Kwifi) * Ffrance * fridgeEquivalence;
const boilerCO2 = conso * (Kdatacenter + Kwifi) * Ffrance * boilerEquivalence;

// Update HTML elements
document.querySelector('#column1 .number-value2:nth-child(1)').textContent = trainCO2.toFixed(2);
document.querySelector('#column1 .number-value2:nth-child(2)').textContent = subwayCO2.toFixed(2);
document.querySelector('#column1 .number-value2:nth-child(3)').textContent = busCO2.toFixed(2);
document.querySelector('#column1 .number-value2:nth-child(4)').textContent = planeCO2.toFixed(2);
document.querySelector('#column2 .number-value2:nth-child(1)').textContent = motorcycleCO2.toFixed(2);
document.querySelector('#column2 .number-value2:nth-child(2)').textContent = carCO2.toFixed(2);
document.querySelector('#column2 .number-value2:nth-child(3)').textContent = fridgeCO2.toFixed(2);
document.querySelector('#column2 .number-value2:nth-child(4)').textContent = boilerCO2.toFixed(2);
