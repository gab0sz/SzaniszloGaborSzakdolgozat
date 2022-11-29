// Complete Project Details at: https://RandomNerdTutorials.com/
const loginElement = document.querySelector('#login-form');
const contentElement = document.querySelector("#content-sign-in");
const userDetailsElement = document.querySelector('#user-details');
const authBarElement = document.querySelector('#authentication-bar');


// Elements for door status
const doorStatusElement = document.getElementById("door-status");
const doorButtonElement = document.getElementById('door-button');
const rfidCheckboxElement = document.getElementById("rfid-checkbox");

// RGB LED element
const rgbLedElement = document.getElementById("color-picker");

// Elements for sensor readings
const tempElement = document.getElementById("temp");
const presElement = document.getElementById("pres");
const humElement = document.getElementById("hum");
const dateElement = document.getElementById("timestamp");
const timeElement = document.getElementById("timestamp2");

// Elements for lights
const kitchenLigthElement = document.getElementById("kitchen-light");
const livingRoomLightElement = document.getElementById("livingroom-light");
const bedroomLightElement = document.getElementById("bedroom-light");
const hallLightElement = document.getElementById("hall-light");
const outSideLightElement = document.getElementById("outside-light");
const pulseElement = document.getElementById("pulse-led");
const changeColorElement = document.getElementById("changing-color");

// Element for camera
const cameraCheckboxElement = document.getElementById("camera-switch");

function plotValues(chart, timestamp, value){
  var x = new Date(Number(timestamp * 1000)).getTime();//.toLocaleDateString() + " " + new Date(Number(timestamp * 1000)).toLocaleTimeString();
  var y = Number (value);
  if(chart.series[0].data.length > 40) {
    chart.series[0].addPoint([x, y], true, true, true);
  } else {
    chart.series[0].addPoint([x, y], true, false, true);
  }
}

const setupUI = (user) => {
  if (user) {
    //toggle UI elements
    loginElement.style.display = 'none';
    contentElement.style.display = 'flex';
    userDetailsElement.style.display ='block';
    userDetailsElement.innerHTML = user.email;

    // get user UID to get data from database
    var uid = user.uid;
    console.log(uid);

    // Database paths (with user UID)
    var dbPath = "UsersData/" + uid.toString() + "/readings";
    var dbPathTemp = 'UsersData/' + uid.toString() + '/readings/latest/temperature';
    var dbPathPres = 'UsersData/' + uid.toString() + '/readings/latest/pressure';
    var dbPathHum = 'UsersData/' + uid.toString() + '/readings/latest/humidity';
    var dbTimeStamp = 'UsersData/' + uid.toString() + '/readings/latest/timestamp';
    var dbPathDoor = 'UsersData/' + uid.toString() + '/door/status';
    var dbPathRfid = 'UsersData/' + uid.toString() + '/door/rfid';
    var dbPathLightsKitchen = 'UsersData/' + uid.toString() + '/lights/kitchen';
    var dbPathLightsLivingRoom = 'UsersData/' + uid.toString() + '/lights/livingroom';
    var dbPathLightsBedroom = 'UsersData/' + uid.toString() + '/lights/bedroom';
    var dbPathLightsHall = 'UsersData/' + uid.toString() + '/lights/hall';
    var dbPathLightsOutSide = 'UsersData/' + uid.toString() + '/lights/outside';
    var dbPathRgbLed = 'UsersData/' + uid.toString() + '/rgbled/rgb';
    var dbPathPulseLed = 'UsersData/' + uid.toString() + '/rgbled/pulse';
    var dbPathChangeColorLed = 'UsersData/' + uid.toString() + '/rgbled/changeColor';
    var dbPathCamera = 'UsersData/' + uid.toString() + '/camera/cameraAllowed';
    // Database references
    var dbRef = firebase.database().ref(dbPath);
    var dbRefTemp = firebase.database().ref().child(dbPathTemp);
    var dbRefPres = firebase.database().ref().child(dbPathPres);
    var dbRefHum = firebase.database().ref().child(dbPathHum);
    var dbRefTimeStamp = firebase.database().ref().child(dbTimeStamp);
    var dbRefDoor = firebase.database().ref().child(dbPathDoor);
    var dbRefRfid = firebase.database().ref().child(dbPathRfid);
    var dbRefLightsKitchen = firebase.database().ref().child(dbPathLightsKitchen);
    var dbRefLightsLivingRoom = firebase.database().ref().child(dbPathLightsLivingRoom);
    var dbRefLightsBedroom = firebase.database().ref().child(dbPathLightsBedroom);
    var dbRefLightsHall = firebase.database().ref().child(dbPathLightsHall);
    var dbRefLightsOutSide = firebase.database().ref().child(dbPathLightsOutSide);
    var dbRefRgbLed = firebase.database().ref().child(dbPathRgbLed);
    var dbRefPulseLed = firebase.database().ref().child(dbPathPulseLed);
    var dbRefChangeColorLed = firebase.database().ref().child(dbPathChangeColorLed);
    var dbRefCamera = firebase.database().ref().child(dbPathCamera);
    var rgbLedValue = 0xff0000;
    var doorStatus = 0;

    // Update page with new readings
    dbRefTemp.on('value', snap => { 
      tempElement.innerText = snap.val()
    });

    dbRefPres.on('value', snap => {
      presElement.innerText = snap.val()
    });

    dbRefHum.on('value', snap => {
      humElement.innerText = snap.val()
    });

    dbRefTimeStamp.on('value', snap => {
      var timestamp = snap.val() * 1000;
      console.log(timestamp);
      var date = new Date(Number(timestamp)).toLocaleDateString();
      console.log(date);
      var time = new Date(Number(timestamp)).toLocaleTimeString();
      dateElement.innerText = date;
      timeElement.innerText = time;
    });
    
    dbRefDoor.on('value', snap => {
      doorStatus = snap.val();
      if(doorStatus == 1){
        doorStatusElement.innerText = "Az ajtó nyitva van";
        doorButtonElement.innerHTML = 'Zárás <i class="fas fa-door-closed"></i>';
       
        doorButtonElement.style.color = "#dc4c64"; 
        doorButtonElement.style.borderColor = "#dc4c64";
      }
      else{
        doorStatusElement.innerText = "Az ajtó zárva van";
        doorButtonElement.innerHTML = 'Nyitás <i class="fas fa-door-open"></i>';
        doorButtonElement.style.color = "#3b71ca"; 
        doorButtonElement.style.borderColor = "#3b71ca";
      }
      console.log(doorStatus);
    });

    dbRefRfid.on('value', snap => {
      var rfidStatus = snap.val();
      if(rfidStatus == 1){
        rfidCheckboxElement.checked = true;
      }
      else{
        rfidCheckboxElement.checked = false;
      }
    });

    rfidCheckboxElement.addEventListener('change', function() {
      console.log("Checkbox clicked");
      if (this.checked) {
        dbRefRfid.set(1);
      } else {
        dbRefRfid.set(0);
      }
    });

    dbRefLightsKitchen.on('value', snap => {
      var kitchenLightStatus = snap.val();
      if(kitchenLightStatus == 1){
        kitchenLigthElement.checked = true;
      }
      else{
        kitchenLigthElement.checked = false;
      }
    });

    kitchenLigthElement.addEventListener('change', function() {
      console.log("Checkbox clicked");
      if (this.checked) {
        dbRefLightsKitchen.set(1);
      } else {
        dbRefLightsKitchen.set(0);
      }
    });

    dbRefLightsLivingRoom.on('value', snap => {
      var livingRoomLightStatus = snap.val();
      if(livingRoomLightStatus == 1){
        livingRoomLightElement.checked = true;
      }
      else{
        livingRoomLightElement.checked = false;
      }
    });

    livingRoomLightElement.addEventListener('change', function() {
      console.log("Checkbox clicked");
      if (this.checked) {
        dbRefLightsLivingRoom.set(1);
      } else {
        dbRefLightsLivingRoom.set(0);
      }
    });

    dbRefLightsBedroom.on('value', snap => {
      var bedroomLightStatus = snap.val();
      if(bedroomLightStatus == 1){
        bedroomLightElement.checked = true;
      }
      else{
        bedroomLightElement.checked = false;
      }
    });

    bedroomLightElement.addEventListener('change', function() {
      console.log("Checkbox clicked");
      if (this.checked) {
        dbRefLightsBedroom.set(1);
      } else {
        dbRefLightsBedroom.set(0);
      }
    });

    dbRefLightsHall.on('value', snap => {
      var hallLightStatus = snap.val();
      if(hallLightStatus == 1){
        hallLightElement.checked = true;
      }
      else{
        hallLightElement.checked = false;
      }
    });

    hallLightElement.addEventListener('change', function() {
      console.log("Checkbox clicked");
      if (this.checked) {
        dbRefLightsHall.set(1);
      } else {
        dbRefLightsHall.set(0);
      }
    });

    dbRefLightsOutSide.on('value', snap => {
      var outSideLightStatus = snap.val();
      if(outSideLightStatus == 1){
        outSideLightElement.checked = true;
      }
      else{
        outSideLightElement.checked = false;
      }
    });

    outSideLightElement.addEventListener('change', function() {
      console.log("Checkbox clicked");
      if (this.checked) {
        dbRefLightsOutSide.set(1);
      } else {
        dbRefLightsOutSide.set(0);
      }
    });

    dbRefCamera.on('value', snap => {
      var cameraStatus = snap.val();
      if(cameraStatus == 1){
        cameraCheckboxElement.checked = true;
      }
      else{
        cameraCheckboxElement.checked = false;
      }
    });

    cameraCheckboxElement.addEventListener('change', function() {
      console.log("Checkbox clicked");
      if (this.checked) {
        dbRefCamera.set(1);
      } else {
        dbRefCamera.set(0);
      }
    });


    doorButtonElement.onclick = () => {
      
      if(doorStatus == 0){
        dbRefDoor.set(1);
        doorStatusElement.innerText = "Az ajtó nyitva van";
        doorButtonElement.innerHTML = 'Zárás <i class="fas fa-door-closed"></i>';
        doorButtonElement.style.color = "#dc4c64"; 
        doorButtonElement.style.borderColor = "#dc4c64";
        console.log("Door button pressed");
      }
      else{
        dbRefDoor.set(0);
        doorStatusElement.innerText = "Az ajtó zárva van";
        doorButtonElement.innerHTML = 'Nyitás <i class="fas fa-door-open"></i>';
        doorButtonElement.style.color = "#3b71ca"; 
        doorButtonElement.style.borderColor = "#3b71ca";
        console.log("Door button pressed");
      }
    };
 

    
    // Delete all data from charts to update with new values when a new range is selected
    //chartT.destroy();

    //chartP.destroy();
    // Render new charts to display new range of data
    chartT = createTemperatureChart();
    chartP = createPressureChart();
    chartH = createHumidityChart();
    // Update the charts with the new range
    // Get the latest readings and plot them on charts (the number of plotted readings corresponds to the chartRange value)
    dbRef.orderByKey().limitToLast(10).on('child_added', snapshot => {
      var jsonData = snapshot.toJSON(); // example: {temperature: 25.02, humidity: 50.20, pressure: 1008.48, timestamp:1641317355}
      // Save values on variables
      var temperature = jsonData.temperature;
      var pressure = jsonData.pressure;
      var humidity = jsonData.humidity;
      var timestamp = jsonData.timestamp;
      // Plot the values on the charts
      plotValues(chartT, timestamp, temperature);
      plotValues(chartP, timestamp, pressure);
      plotValues(chartH, timestamp, humidity);
    });
    
    rgbLedElement.addEventListener('change', function() {
      rgbLedValue = rgbLedElement.value;
      dbRefRgbLed.set(rgbLedValue);
      console.log(rgbLedValue);
    });

    dbRefRgbLed.on('value', snap => {
      rgbLedValue = snap.val();
      rgbLedElement.value = rgbLedValue;
    });

    dbRefPulseLed.on('value', snap => {
      var pulseLedStatus = snap.val();
      if(pulseLedStatus == 1){
        pulseElement.checked = true;
      }
      else{
        pulseElement.checked = false;
      }
    });

    pulseElement.addEventListener('change', function() {
      console.log("Checkbox clicked");
      if (this.checked) {
        dbRefPulseLed.set(1);
      } else {
        dbRefPulseLed.set(0);
      }
    });

    dbRefChangeColorLed.on('value', snap => {
      var changeColorLedStatus = snap.val();
      if(changeColorLedStatus == 1){
        changeColorElement.checked = true;
      }
      else{
        changeColorElement.checked = false;
      }
    });

    changeColorElement.addEventListener('change', function() {
      console.log("Checkbox clicked");
      if (this.checked) {
        dbRefChangeColorLed.set(1);
      } else {
        dbRefChangeColorLed.set(0);
      }
    });

    




  // IF USER IS LOGGED OUT
  } else{
    // toggle UI elements
    user = null;
    loginElement.style.display = 'flex';
    userDetailsElement.style.display ='none';
    contentElement.style.display = 'none';
  }

}

