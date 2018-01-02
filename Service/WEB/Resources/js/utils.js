/*
 * Copyright (C) 2014 zRed Comunicaciones y Automatización S.L.
 * See the file "LICENSE" for the full license governing this code.
 */
var changeAlert = false;

var breakpointDefinition = {
    tablet: 1024,
    phone: 480
};

var dtPaginateSimple = {
    "sNext": '<i class="fa fa-chevron-right"></i>',
    "sPrevious": '<i class="fa fa-chevron-left"></i>'
};

function displayChangeAlert() {
    changeAlert = true;
    $('.change-alert').slideDown(500);
}

function exitConfirmation() {
    if (changeAlert)
        return 'Has realizado modificaciones, con esta acción perderás las modificaciones realizadas.';
}

$('.discardChanges').click(function () {
    location.reload(true);
});

$('.refresh').click(function () {
    location.reload(true);
});

window.onbeforeunload = exitConfirmation;

function hideChangeAlert() {
    changeAlert = false;
    $('.change-alert').slideUp(500);
}
function formatUndefined(text) {
    if (text === undefined) {
        return '';
    }
    return text;
}
var downloadURL = function downloadURL(url) {
    var hiddenIFrameID = 'hiddenDownloader',
            iframe = document.getElementById(hiddenIFrameID);
    if (iframe === null) {
        iframe = document.createElement('iframe');
        iframe.id = hiddenIFrameID;
        iframe.style.display = 'none';
        document.body.appendChild(iframe);
    }
    iframe.src = url;
};

jQuery.fn.toggleText = function (value1, value2) {
    return this.each(function () {
        var $this = $(this),
                text = $this.text();

        if (text.indexOf(value1) > -1)
            $this.text(text.replace(value1, value2));
        else
            $this.text(text.replace(value2, value1));
    });
};

$.fn.serializeObject = function ()
{
    var o = {};
    var a = this.serializeArray();
    $.each(a, function () {
        if (o[this.name]) {
            if (!o[this.name].push) {
                o[this.name] = [o[this.name]];
            }
            o[this.name].push(this.value || '');
        } else {
            o[this.name] = this.value || '';
        }
    });
    return o;
};

String.prototype.format = function () {
    var literal = this;

    for (var i = 0; i < arguments.length; i++) {
        var regex = new RegExp('\\{' + i + '\\}', 'g');
        literal = literal.replace(regex, arguments[i]);
    }

    return literal;
};

function clone(array) {
    var cloned = [];
    for (var i = 0; i < array.length; i++) {
        cloned[i] = [];
        for (var j = 0; j < array[i].length; j++) {
            cloned[i][j] = array[i][j];
        }
    }
    return cloned;
}

function fillForm(data, form) {
    $.each(data, function (name, val) {
        var $el = $('[name="' + name + '"]', form);
        if ($el.length > 0) {
            var type = $el.attr('type');
            switch (type) {
                case 'checkbox':
                    if ($el.length == 1) {//Checkbox unique
                        if (val == 1) {
                            $el.attr('checked', 'checked');
                        } else {
                            $el.removeAttr('checked');
                        }
                        break;
                    } else if ($el.length > 1) {
                        for (var i in val) {
                            $el.filter('[value="' + val[i] + '"]').attr('checked', 'checked');
                        }
                    }
                case 'radio':
                    $el.filter('[value="' + val + '"]').attr('checked', 'checked');
                    break;
                default:
                    var tag = $el[0].nodeName;
                    if (tag === 'SELECT') {
                        val = val == null ? 'NULL' : val;
                    }
                    if ($el.attr('formatFunction') !== undefined) {
                        val = window[$el.attr('formatFunction')](val);
                    }
                    $el.val(val);
                    if (tag === 'SELECT') {
                        $el.change();
                    }
            }
        }
    });
}

function fillDiv(data, div) {
    $.each(data, function (name, val) {
        var $el = $('[data-name="' + name + '"]', div);
        if ($el.length > 0) {
            if ($el.attr('data-formatFunction') !== undefined) {
                val = window[$el.attr('data-formatFunction')](val);
            }
            $el.text(val);
        }
    });
}

function cleanForm(form) {
    if ($(form).data('bootstrapValidator') !== undefined) {
        $(form).data('bootstrapValidator').resetForm();
    }
    var data = form.serializeObject();
    $.each(data, function (name, val) {
        var $el = $('[name="' + name + '"]', form),
                type = $el.attr('type'),
                defaultValue = $el.attr('default');
        switch (type) {
            case 'checkbox':
            case 'radio':
                $el.filter('[value="' + val + '"]').removeAttr('checked');
                break;
            case 'number':
                $el.val(defaultValue || 0);
            default:
                var tag = $el[0].nodeName;
                if (tag === 'SELECT') {
                    if (defaultValue === undefined) {
                        $el[0].selectedIndex = 0;
                    } else {
                        $el.val(defaultValue || '');
                    }
                } else {
                    $el.val(defaultValue || '');
                }
        }
    });
}

var typewatch = (function () {
    var timer = 0;
    return function (callback, ms) {
        clearTimeout(timer);
        timer = setTimeout(callback, ms);
    }
})();


function roundTo(num, pos) {
    if (num < 0.00001 && num > 0) {
        return 0;
    }
    return +(Math.round(num + "e+" + pos) + "e-" + pos);
}

function formatTimeUnit(number) {

    var base = parseInt(getBaseLog(10, number) / 3);

    if (base >= 2) {
        return roundTo(number / 1000000., 2) + ' s';
    }
    if (base == 1) {
        return roundTo(number / 1000., 2) + ' ms';
    }
    if (base == 0) {
        return number + ' µ';
    }
}

var intVal = function (i) {
    if (typeof i === 'string') {
        var val = $(i).val();
        if (val === undefined) {
            return i.replace(/[\€,]/g, '') * 1
        }
        return val === '' ? 0 : parseFloat(val);
    } else if (typeof i === 'number') {
        return i;
    } else {
        return 0;
    }
    return typeof i === 'string' ?
            val.replace(/[\€,]/g, '') * 1 :
            typeof i === 'number' ?
            val : 0;
};

//Get highlight color from color
function highlight(col, amt) {

    var usePound = false;

    if (col[0] == "#") {
        col = col.slice(1);
        usePound = true;
    }

    var num = parseInt(col, 16);

    var r = (num >> 16) + amt;

    if (r > 255)
        r = 255;
    else if (r < 0)
        r = 0;

    var b = ((num >> 8) & 0x00FF) + amt;

    if (b > 255)
        b = 255;
    else if (b < 0)
        b = 0;

    var g = (num & 0x0000FF) + amt;

    if (g > 255)
        g = 255;
    else if (g < 0)
        g = 0;

    return (usePound ? "#" : "") + (g | (b << 8) | (r << 16)).toString(16);

}

function formatDate(date) {
    return moment(date, 'YYYY-MM-DD HH:mm:ssZ').format('DD/MM/YYYY');
}

function formatDateTime(date) {
    return moment(date, 'YYYY-MM-DD HH:mm:ssZ').format('DD/MM/YYYY HH:mm:ss');

}

$('body').on('click', 'div.btn-group .dropdown-menu a', function (event) {
    event.preventDefault();
});

function UrlExists(url)
{
    var http = new XMLHttpRequest();
    http.open('HEAD', url, false);
    http.send();
    return http.status != 404;
}

function descargarDocumento(documento) {
    if (documento.id_documento.split('_')[0] === 'new') {
        alert('Debe guardar los cambios antes de poder descargar este fichero.');
    } else {
        window.open(
                'productor/descargar/' + documento.id_documento,
                '_blank'
                );
    }
}

function getCampanya(fechaPlantacion) {
    var year = '?';
    if (fechaPlantacion && fechaPlantacion !== '?') {
        fechaPlantacion = moment(fechaPlantacion, "DD/MM/YYYY");
        year = fechaPlantacion.year() + (fechaPlantacion.month() >= 9 ? 1 : 0);
    }
    return year;
}



// Returns the ISO week of the date.
Date.prototype.getWeek = function () {
    var date = new Date(this.getTime());
    date.setHours(0, 0, 0, 0);
    // Thursday in current week decides the year.
    date.setDate(date.getDate() + 3 - (date.getDay() + 6) % 7);
    // January 4 is always in week 1.
    var week1 = new Date(date.getFullYear(), 0, 4);
    // Adjust to Thursday in week 1 and count number of weeks from date to week1.
    return 1 + Math.round(((date.getTime() - week1.getTime()) / 86400000
            - 3 + (week1.getDay() + 6) % 7) / 7);
}

// Returns the four-digit year corresponding to the ISO week of the date.
Date.prototype.getWeekYear = function () {
    var date = new Date(this.getTime());
    date.setDate(date.getDate() + 3 - (date.getDay() + 6) % 7);
    return date.getFullYear();
}


function getWeekNumber(d) {
    // Copy date so don't modify original
    d = new Date(+d);
    d.setHours(0, 0, 0);
    // Set to nearest Thursday: current date + 4 - current day number
    // Make Sunday's day number 7
    d.setDate(d.getDate() + 4 - (d.getDay() || 7));
    // Get first day of year
    var yearStart = new Date(d.getFullYear(), 0, 1);
    // Calculate full weeks to nearest Thursday
    var weekNo = Math.ceil((((d - yearStart) / 86400000) + 1) / 7)
    // Return array of year and week number
    return [d.getFullYear(), weekNo];
}

function weeksInYear(year) {
    var d = new Date(year, 11, 31);
    var week = getWeekNumber(d)[1];
    return week == 1 ? getWeekNumber(d.setDate(24))[1] : week;
}

function getBaseLog(x, y) {
    return Math.log(y) / Math.log(x);
}



