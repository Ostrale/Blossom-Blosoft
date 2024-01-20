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

    const conso = 38 * 10**9; // 38Gb in bytes
    const consoCO2 = conso * (Kdatacenter + Kwifi) * Ffrance; // g CO2

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
    document.getElementById("conso").innerHTML = consoCO2.toFixed(2);
});