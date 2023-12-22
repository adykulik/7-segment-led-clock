$(document).ready(function () {
    if ($('#inline').length) {
        $('#inline').minicolors({
            inline: $('#inline').attr('data-inline') === 'true',
            theme: 'bootstrap'
        });
    }

    if ($('#currentDateTime').length) {
        window.setInterval(showDateTime, 900);
    }
});

function showDateTime() {
    console.log(1);
    var currentDateTime = $('#currentDateTime').get(0);
    currentDateTime.innerHTML = moment().format('MMMM DD YYYY HH:mm:ss');
}

function setColor() {
    var color = $('#inline').minicolors('rgbObject');
    console.log(color);
    $.post("/color", color);
}

function setBrightness() {
    var brightness = $('#rangeBrightness').val();
    console.log(brightness);
    $.post("/brightness", { brightness: brightness });
}

function setDateTime() {
    var d = moment().format('MMMM DD YYYY');
    var t = moment().format('HH:mm:ss');
    var datetime = { date: d, time: t };
    console.log(datetime);
    $.post("/setdate", datetime);
}

function showTemperature() {
    var c = $('#temperatureCorrection').val();
    var d = $('#temperatureDisplay').val();
    var temperature = { correction: c, symbol: d };
    $.post("/temperature", temperature);
}

function showClock() {
    $.post("/clock");
}

function updateHourFormat() {
    var hourformat = $('#hourFormat').val();
    $.post("/hourformat", { hourformat: hourformat });
}

function switchCheck(){
    var autoCheck = $('#switch').is(':checked');
    if(autoCheck){
        $('#rangeBrightness').hide();
        $('#buttonBri').hide();
    }
    else{
        $('#rangeBrightness').show();
        $('#buttonBri').show();
    }
    $.post("/autoCheck", {auto: autoCheck});
}