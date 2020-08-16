window.onload = function() {
    load_shelf();
    load_shelf_status();
    load_commands();
};
window.setInterval(function() {
    // load_shelf_status();
}, 200);


function load_shelf_status() {
    var url = "api/read_last_ambient.php"
    $.getJSON(url, function(response) {
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

function load_shelf() {
    var url = "api/read_units_products.php"
    $.getJSON(url, function(response) {
        for (let index = 0; index < response["units"].length; index++) {
            const unit = response["units"][index];
            // $(".pic.unit".concat(unit["id"])).attr("src", "img/placeholder.png");
            $(".pic.unit".concat(unit["id"])).attr("src", "img/".concat(unit["image"]));
            $(".pic.unit".concat(unit["id"])).attr("alt", unit["name"]);
            $("#unit-info".concat(unit["id"])).html(unit["name"]);
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

function update_commands(component_id) {
    var url = "api/update_commands.php";
    var checkBox = document.getElementById(component_id);
    var status = 0;
    if (checkBox.checked == true)
    status = 1;
    $.post(url, { [component_id]: status }, function(result) {
        console.log(result);
    });
}

function show_products(unitId) {
    var url = `products.html?u=${unitId}`;
    window.open(url, "_self");
}