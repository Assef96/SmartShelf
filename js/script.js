var units_product_id = [0, 0, 0, 0, 0];


window.onload = function() {
    load_shelf_status();
    load_shelf();
    load_commands();
    // load_products_list();
    // load_product();
};
window.setInterval(function() {
    // load_ambient();
    // load_shelf_status();
}, 200);


function load_shelf_status() {
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
        if (timeDifference < 10000)
            $("#connection-status").html("Connected");
        else
            $("#connection-status").html("Not Connected");
    });

    var url = "api/read_sensors.php"
    $.getJSON(url, function(response) {
        for (let index = 0; index < response["sensors"].length; index++) {
            const sensor = response["sensors"][index];
            const id = parseInt(sensor["id"]);
            var status = parseInt(sensor["status"]);
            if (id <= 4) {
                if (status == 0)
                    $("#pic".concat(id)).addClass("transparent");
                else
                    $("#pic".concat(id)).removeClass("transparent");
            }
            if (id == 5) {
                if (status > 4)
                    status = 4;
                for (let i = 5; i < 5 + status; i++)
                    $("#pic".concat(i)).removeClass("transparent");
                for (let i = 5 + status; i <= 8; i++)
                    $("#pic".concat(i)).addClass("transparent");
            }
            if (id >= 6 && id <= 10) {
                if (status == 0)
                    $("#pic".concat(id + 3)).addClass("transparent");
                else
                    $("#pic".concat(id + 3)).removeClass("transparent");
            }
            if (id == 11) {
                if (status > 3)
                    status = 3;
                for (let i = 14; i < 14 + status; i++)
                    $("#pic".concat(i)).removeClass("transparent");
                for (let i = 14 + status; i <= 16; i++)
                    $("#pic".concat(i)).addClass("transparent");
            }
            if (id == 12) {
                if (status > 3)
                    status = 3;
                for (let i = 17; i < 17 + status; i++)
                    $("#pic".concat(i)).removeClass("transparent");
                for (let i = 17 + status; i <= 19; i++)
                    $("#pic".concat(i)).addClass("transparent");
            }
        }
    });
}

function load_commands() {
    var url = "api/read_commands.php"
    $.getJSON(url, function(response) {
        var lamp1 = (response['commands']['lamp1']);
        if (lamp1 == '1') {
            $("#lamp1").prop("checked", true);
        }
        var lamp2 = (response['commands']['lamp2']);
        if (lamp2 == '1') {
            $("#lamp2").prop("checked", true);
        }
        var buzzer = (response['commands']['buzzer']);
        if (buzzer == '1') {
            $("#buzzer").prop("checked", true);
        }
    });
}

function load_shelf() {
    var url = "api/read_units_products.php"
    $.getJSON(url, function(response) {
        for (let index = 0; index < response["units"].length; index++) {
            const unit = response["units"][index];
            // $(".pic.unit".concat(unit["id"])).attr("src", "img/placeholder.png");
            $(".pic.unit".concat(unit["id"])).attr("src", "img/".concat(unit["image"]));
            $(".pic.unit".concat(unit["id"])).attr("alt", unit["name"]);
            $("#unit-info".concat(unit["id"])).html(unit["name"]);
            units_product_id[index] = unit["product_id"];
        }
    });
}

function load_products_list() {
    var url = "api/read_products.php"
    $.getJSON(url, function(response) {
        var str = '';
        for (let index = 0; index < response["products"].length; index++) {
            const product = response["products"][index];
            const id = product["id"];
            const name = product["name"];
            const price = product["price"];
            const image = product["image"];

            // $(".pic.unit".concat(unit["id"])).attr("src", "img/placeholder.png");
            // $(".pic.unit".concat(unit["id"])).attr("src", "img/".concat(unit["image"]));
            // $(".pic.unit".concat(unit["id"])).attr("alt", unit["name"]);
            // $("#unit-info".concat(unit["id"])).html(unit["name"]);
            str += `<div class="card" style="width: 15rem;">
            <img src="img/${image}" class="card-img-top" alt="${image}">
            <div class="card-body">
            <h5 class="card-title">${name}</h5>
            <p class="card-text">id: ${id}</p>
            <p class="card-text">Price: ${price}</p>
            <a href="#" class="btn btn-primary">Go somewhere</a>
            </div>
            </div>`;
            $("#products-list").html(str);
        }
    });
}




$('#lamp1').on('click', function() {
    var url = "api/update_commands.php";
    var checkBox = document.getElementById("lamp1");
    var status = 0;
    if (checkBox.checked == true)
        status = 1;
    $.post(url, { "lamp1": status }, function(result) {
        console.log(result);
    });
});

$('#lamp2').on('click', function() {
    var url = "api/update_commands.php";
    var checkBox = document.getElementById("lamp2");
    var status = 0;
    if (checkBox.checked == true)
        status = 1;
    $.post(url, { "lamp2": status }, function(result) {
        console.log(result);
    });
});

$('#buzzer').on('click', function() {
    var url = "api/update_commands.php";
    var checkBox = document.getElementById("buzzer");
    var status = 0;
    if (checkBox.checked == true)
        status = 1;
    $.post(url, { "buzzer": status }, function(result) {
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

$('#alarm-off').on('click', function() {
    var url = "api/update_actuators.php?alarm=off";
    $.getJSON(url, function(response) {
        console.log(response);
    });
});


function show_products(unitId) {
    var url = `products.html?u=${unitId}`;
    window.open(url, "_self");
}