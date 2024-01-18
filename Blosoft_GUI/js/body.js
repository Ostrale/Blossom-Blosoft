document.write(`
<style>
    body {
        background-image: url('image/forest-6761846.jpg');
        background-repeat: no-repeat;
    }
    </style>
<div class="sidebar">
<div class="logo-details">
      <i class='bx icon'><img class='bx icon'src="image/logo.png"></i>
        <div class="logo_name">Blosoft</div>
        <i class='bx bx-menu' id="btn" ></i>
</div>
<ul class="nav-list">
      <li>
        <a href="#", class="dashboardBtn">
          <i class='bx bx-grid-alt'></i>
          <span class="links_name">Dashboard</span>
        </a>
         <span class="tooltip">Dashboard</span>
      </li>
     <li>
      <a href="#", class="analyticsBtn">
        <i class='bx bx-line-chart'></i>
         <span class="links_name">Analytics</span>
       </a>
       <span class="tooltip">Analytics</span>
     </li>
     <li>
        <a href="#", class="analyticsBtn2">
           <i class='bx bxs-pie-chart-alt-2'></i>
           <span class="links_name">Analytics division</span>
         </a>
         <span class="tooltip">Analytics division</span>
       </li>
      <li>
       <a href="#", class="analyticsBtn3">
          <i class='bx bxs-bus'></i>
          <span class="links_name">Analytics comparison</span>
        </a>
        <span class="tooltip">Analytics comparison</span>
      </li>
     <li>
       <a href="#", class="settingBtn">
         <i class='bx bx-cog' ></i>
         <span class="links_name">Settings</span>
       </a>
       <span class="tooltip">Settings</span>
     </li>
     <li class="profile">
        <div class="logo_name">Blosoft By Vert-I-Good</div>
             <i class='bx bx-log-out' id="log_out" ></i>
     </li>
</ul>
</div>
`
   
)