/*
 * Copyright 2010 Ovidiu Predescu <ovidiu@gmail.com>
 * Date: December 2010
 */
String.prototype.trim = function () {
    return this.replace(/^\s*/, "").replace(/\s*$/, "");
};

function Button(elem) {
    var btn = this;
    this.elem = elem;
    this.setEnabled(false);
    this.elem.click(function(e) {
	e.preventDefault();
	btn.clickHandler(btn, e);
    });
};

Button.prototype.setEnabled = function(yesno) {
    this.enabled = yesno;
    this.elem.attr('class', function(idx, val) {
	var newClass = yesno ? "on" : "off";
	return newClass;
    });
};

Button.prototype.isEnabled = function() { return this.enabled; };

Button.prototype.clickHandler = function(btn, e) {
    var url = btn.elem.attr('href');
    $.ajax({type: "POST",
	    data: (!btn.isEnabled() + 0).toString(10),
	    dataType: "text",
	    cache: false,
	    url: url,
	    success: function(r) {
	    },
	    error: function(s, xhr, status, e) {
		console.log("POST failed: " + s.responseText);
	    }
	   });
};

function ledStatus(btn, url) {
    $.ajax({type: "GET",
	    cache: false,
	    url: url,
	    success: function(status) {
		status = parseInt(status.trim());
		btn.setEnabled(status);
		window.setTimeout(function() {ledStatus(btn, url);}, 400);
	    },
	    error: function(s, xhr, status, e) {
		console.log("Getting status failed: "
			    + s.responseText);
	    }
	   });
};

$(document).ready(
    function() {
	var lightBulb = new Button($("#lightbulb"));
	ledStatus(lightBulb, "/ledstatus");
    });
