window.onload = function() {
    load_ambient();
    // load_shelf();
    load_commands();
};
window.setInterval(function() {
    load_ambient();
    load_shelf();
}, 2000);

function load_ambient() {
    var url = "api/read_last_ambient.php"
    $.getJSON(url, function(response) {
        // var val = response;
        var humidity = (response['ambient']['humidity']);
        var temperature = (response['ambient']['temperature']);
        var light = (response['ambient']['light']);
        var time = (response['ambient']['time']);
        $("#temperature").html(temperature);
        $("#humidity").html(humidity);
        $("#light").html(light);
        var timeDifference = Date.now() - Date.parse(time);
        if(timeDifference < 10000)
            $("#connection_status").html("Connected");
        else
            $("#connection_status").html("Not Connected");


    });
}

function load_commands() {
    var url = "api/read_commands.php"
    $.getJSON(url, function(response) {
        var lamp1 = (response['commands']['lamp1']);
        if (lamp1 == '1'){
            $("#lamp1").prop("checked", true);
        }
        var lamp2 = (response['commands']['lamp2']);
        if (lamp2 == '1'){
            $("#lamp2").prop("checked", true);
        }
        var buzzer = (response['commands']['buzzer']);
        if (buzzer == '1'){
            $("#buzzer").prop("checked", true);
        }
    });
}

function load_shelf() {
    var url = "api/read_units.php"
    $.getJSON(url, function(response) {
        for (let index = 0; index < response["units"].length; index++) {
            const product = response["units"][index];
            $("#product_name".concat(product["id"])).html(product["name"]);
            $("#product_image".concat(product["id"])).attr("src", "img/".concat(product["image"]));
            $("#product_image".concat(product["id"])).attr("alt", product["name"]);
            if (product["status"] == 0) {
                $("#product_image".concat(product["id"])).addClass("transparent");
                // $("#product_image".concat(product["id"])).addClass("visible");
            } else {
                $("#product_image".concat(product["id"])).removeClass("transparent");
                // $("#product_image".concat(product["id"])).removeClass("visible");
            }
        }
    });
}


$('#lamp1').on('click', function() {
    var url = "api/update_commands.php";
    var checkBox = document.getElementById("lamp1");
    var status = 0;
    if (checkBox.checked == true)
        status = 1;
    $.post(url, {"lamp1": status}, function(result){
      console.log(result);
    });
});

$('#lamp2').on('click', function() {
    var url = "api/update_commands.php";
    var checkBox = document.getElementById("lamp2");
    var status = 0;
    if (checkBox.checked == true)
        status = 1;
    $.post(url, {"lamp2": status}, function(result){
      console.log(result);
    });
});

$('#buzzer').on('click', function() {
    var url = "api/update_commands.php";
    var checkBox = document.getElementById("buzzer");
    var status = 0;
    if (checkBox.checked == true)
        status = 1;
    $.post(url, {"buzzer": status}, function(result){
      console.log(result);
    });
});
  

$('#led-on').on('click', function() {
    var url = "api/update_actuators.php?status=on";
    $.getJSON(url, function(response) {
        console.log(response);
    });
});

$('#led-off').on('click', function() {
    var url = "api/update_actuators.php?status=off";
    $.getJSON(url, function(response) {
        console.log(response);
    });
});

$('#alarm-on').on('click', function() {
    var url = "api/update_actuators.php?alarm=on";
    $.getJSON(url, function(response) {
        console.log(response);
    });
});

$('#alarm-off').on('click', function() {
    var url = "api/update_actuators.php?alarm=off";
    $.getJSON(url, function(response) {
        console.log(response);
    });
});

// $("#insert_product_form").submit(function(event) {
//     event.preventDefault(); //prevent default action 
//     var url = $(this).attr("action"); //get form action url
//     var form_data = $(this).serialize(); //Encode form elements for submission
//     var enctype = $(this).attr("enctype"); //get form action url

//     // $.post(url, form_data, function(response) {
//     //     console.log(response);
//     // });

//     $.ajax({
//         method: 'POST',
//         enctype: enctype,
//         url: url,
//         data: form_data,
//         cache: false,
//         contentType: false,
//         processData: false,
//         timeout: 600000,
//         success: function(response) {
//             console.log(response);
//         }
//     });
// });