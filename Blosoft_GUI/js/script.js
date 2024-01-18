let sidebar = document.querySelector(".sidebar");
let closeBtn = document.querySelector("#btn");
let settingsBtn = document.querySelector(".settingBtn");
let dashboardBtn = document.querySelector(".dashboardBtn");
let analyticsBtn = document.querySelector(".analyticsBtn");
let analyticsBtn2 = document.querySelector(".analyticsBtn2");
let analyticsBtn3 = document.querySelector(".analyticsBtn3");
let log_outBtn = document.querySelector(".bx-log-out");

closeBtn.addEventListener("click", ()=>{
    sidebar.classList.toggle("open");
    menuBtnChange();//calling the function(optional)
});

// following are the code to change sidebar button(optional)
function menuBtnChange() {
   if(sidebar.classList.contains("open")){
     closeBtn.classList.replace("bx-menu", "bx-menu-alt-right");//replacing the iocns class
   }else {
     closeBtn.classList.replace("bx-menu-alt-right","bx-menu");//replacing the iocns class
   }
}

settingsBtn.addEventListener("click", ()=>{
    let time = 0;
    if(sidebar.classList.contains("open")){sidebar.classList.toggle("open");time=500;}
    setTimeout(() => {
      window.open("settings.html", "_self");
    }, time);
    
});

dashboardBtn.addEventListener("click", ()=>{
    let time = 0;
    if(sidebar.classList.contains("open")){
      sidebar.classList.toggle("open");time=500;}
    setTimeout(() => {
      window.open("index.html", "_self");
    }, time);
});

analyticsBtn.addEventListener("click", ()=>{
    let time = 0;
    if(sidebar.classList.contains("open")){sidebar.classList.toggle("open");time=500;}
    setTimeout(() => {
      window.open("analytics.html", "_self");
    }, time);
  
});

analyticsBtn2.addEventListener("click", ()=>{
  let time = 0;
  if(sidebar.classList.contains("open")){sidebar.classList.toggle("open");time=500;}
  setTimeout(() => {
    window.open("analytics2.html", "_self");
  }, time);

});

analyticsBtn3.addEventListener("click", ()=>{
  let time = 0;
  if(sidebar.classList.contains("open")){sidebar.classList.toggle("open");time=500;}
  setTimeout(() => {
    window.open("analytics3.html", "_self");
  }, time);

});

log_outBtn.addEventListener("click", ()=>{
    window.close();
});