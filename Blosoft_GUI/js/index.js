
function getRandomInt(max) {
    return Math.floor(Math.random() * max);
}

var pourcentage = getRandomInt(101);
if (pourcentage < 10){
    var Pourcentage_entier = 0;
}else if (pourcentage == 100){
    var Pourcentage_entier = 9;
}else{
    var Pourcentage_texte = pourcentage.toString();
    var Pourcentage_texte = Pourcentage_texte.slice(0, 1);
    var Pourcentage_entier = parseInt(Pourcentage_texte);
}

let var3 = "<img src='image/arbre_" + Pourcentage_entier + ".png 'alt='Photo dun arbre'/>";
console.log(var3);
let imageinp = document.getElementById("arbre");
imageinp.innerHTML = var3;

let ecoscore = document.getElementById("ecoscore");
ecoscore.innerHTML = "Your Ecoscore : "+pourcentage;