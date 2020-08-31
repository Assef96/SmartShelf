window.unitId = 0;
var productId;
window.onload = function () {
    load_products_list();
};

function load_products_list() {
    var parameters = location.search.substring(1).split("&");
    var temp = parameters[0].split("=");
    window.unitId = unescape(temp[1]);
    var url = "api/read_products.php"
    $.getJSON(url, function (response) {
        var str = `<div class="col-lg-2 col-md-3 col-6 px-1 mb-2" style="height: 340px;">
                    <div class="card text-white bg-dark h-100">
                        <span class="icon card-img" style="margin: auto;" onclick="edit_product(0)" data-toggle="modal" data-target="#exampleModal">
      <i class="fas fa-plus-circle insert-icon"></i></span>
                    </div>
                    </div>`;
        for (let index = 0; index < response["products"].length; index++) {
            const product = response["products"][index];
            const id = product["id"];
            const name = product["name"];
            const price = product["price"];
            const image = product["image"];
            str += `<div class="col-lg-2 col-md-3 col-6 px-1 mb-2">
                        <div class="card text-white bg-dark h-100">
                            <img src="img/${image}" class="card-img-top p-2 m-0" alt="${image}">
                            <div>
                                <h5 class="card-title m-1">${name}</h5>
                                <p class="card-text m-1">Product ${id}</p>
                                <p class="card-text m-1 mb-3">${price} $</p>
                                <div class="card-footer">
                                    <div class="row">
                                        <div class="col-4">
                                            <span onclick="select_product(${id})" class="icon"><i class="fas fa-check-square icon-button"></i></span>
                                        </div>
                                        <div class="col-4">
                                            <span  onclick="edit_product(${id})"class="icon" data-toggle="modal" data-target="#exampleModal"><i class="fas fa-edit icon-button"></i></span>
                                        </div>
                                        <div class="col-4">
                                            <span  onclick="delete_product(${id})" class="icon"><i class="fas fa-trash icon-button"></i></span>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                        </div>
                    </div>`;
            $("#products-list").html(str);
        }
    });
}

function edit_product(i) {
    productId = i;
    if (i == 0) {
        $("#image-preview").attr("src", "img/placeholder.png");
        $("#name").attr("value", "");
        $("#price").attr("value", "");
        return;
    }
    var url = `api/read_product.php?id=${productId}`;
    $.getJSON(url, function (response) {
        const product = response["product"];
        $("#image-preview").attr("src", `img/${product["image"]}`);
        $("#name").attr("value", product["name"]);
        $("#price").attr("value", product["price"]);
    });
}

function delete_product(productId) {
    var url = "api/delete_product.php";
    var form_data = { "id": productId };
    $.post(url, form_data, function (result) {
        console.log(result);
    });
    location.reload();
}

$("#product-form").submit(function (event) {
    event.preventDefault();
    var enctype = $(this).attr("enctype");
    var url = `api/edit_or_insert_product.php?id=${productId}`
    var form_data = new FormData(this);

    $.ajax({
        method: 'POST',
        enctype: enctype,
        url: url,
        data: form_data,
        cache: false,
        contentType: false,
        processData: false,
        timeout: 600000,
        success: function (response) {
            console.log(response);
            edit_product();
            location.reload();
        },
        error: function (response) {
            console.log(response);
        }
    });
});

function select_product(productId) {
    var url = "api/set_product.php";
    var form_data = { "unitId": window.unitId, "productId": productId };
    $.post(url, form_data, function (result) {
        console.log(result);
    });
    window.open("..", "_self");
}
