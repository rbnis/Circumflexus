var initialized = false;
var options = {};

function GColorFromHex(hex) {
	var hexNum = parseInt(hex, 16);
	var a = 192;
	var r = (((hexNum >> 16) & 0xFF) >> 6) << 4;
	var g = (((hexNum >>  8) & 0xFF) >> 6) << 2;
	var b = (((hexNum >>  0) & 0xFF) >> 6) << 0;
	return a + r + g + b;
}
function GColorToHex(color) {
	var r = (color & 48) >> 4;
	var g = (color & 12) >> 2;
	var b = (color & 3) >> 0;
	var hexString = [r,g,b].map(function (x) {
		x *= 5;
		return x.toString(16) + x.toString(16);
	}).join('');
	return hexString.toUpperCase();
}

Pebble.addEventListener("ready", function(e) {
	//console.log("PblKit_called --- ready");
	initialized = true;
});

Pebble.addEventListener("showConfiguration", function(e) {
	//console.log("PblKit_called --- showConfiguration");
	var localoptions = JSON.parse(localStorage.getItem("localOptions"));
	console.log('http://robinhuett.github.io/TimeCirc/config.html' + '?' +
				'color_text_select=' + localoptions.KEY_COLOR_TEXT + '&' + 
				'color_indicator_select=' + localoptions.KEY_COLOR_INDICATOR + '&' + 
				'color_ring_1_select=' + localoptions.KEY_COLOR_RING_1 + '&' + 
				'color_ring_2_select=' + localoptions.KEY_COLOR_RING_2 + '&' + 
				'color_bg_in_select=' + localoptions.KEY_COLOR_BG_IN + '&' + 
				'color_bg_out_select=' + localoptions.KEY_COLOR_BG_OUT + '&' + 
				'show_day_select=' + localoptions.KEY_SHOW_DAY + '&' + 
				'show_date_select=' + localoptions.KEY_SHOW_DATE + '&' + 
				'vibe_bt_select=' + localoptions.KEY_VIBE_BT);
	Pebble.openURL('http://robinhuett.github.io/TimeCirc/config.html' + '?' +
				'color_text_select=' + localoptions.KEY_COLOR_TEXT + '&' + 
				'color_indicator_select=' + localoptions.KEY_COLOR_INDICATOR + '&' + 
				'color_ring_1_select=' + localoptions.KEY_COLOR_RING_1 + '&' + 
				'color_ring_2_select=' + localoptions.KEY_COLOR_RING_2 + '&' + 
				'color_bg_in_select=' + localoptions.KEY_COLOR_BG_IN + '&' + 
				'color_bg_out_select=' + localoptions.KEY_COLOR_BG_OUT + '&' + 
				'show_day_select=' + localoptions.KEY_SHOW_DAY + '&' + 
				'show_date_select=' + localoptions.KEY_SHOW_DATE + '&' + 
				'vibe_bt_select=' + localoptions.KEY_VIBE_BT);
});

Pebble.addEventListener("webviewclosed", function(e) {
	//console.log("PblKit_called --- webviewclosed");
	//Using primitive JSON validity and non-empty check
	if (e.response.charAt(0) == "{" && e.response.slice(-1) == "}" && e.response.length > 5) {
		options = JSON.parse(decodeURIComponent(e.response));
		
		options.KEY_COLOR_TEXT = GColorFromHex(options.KEY_COLOR_TEXT.substring(2));
		options.KEY_COLOR_INDICATOR = GColorFromHex(options.KEY_COLOR_INDICATOR.substring(2));
		options.KEY_COLOR_RING_1 = GColorFromHex(options.KEY_COLOR_RING_1.substring(2));
		options.KEY_COLOR_RING_2 = GColorFromHex(options.KEY_COLOR_RING_2.substring(2));
		options.KEY_COLOR_BG_IN = GColorFromHex(options.KEY_COLOR_BG_IN.substring(2));
		options.KEY_COLOR_BG_OUT = GColorFromHex(options.KEY_COLOR_BG_OUT.substring(2));
		
		Pebble.sendAppMessage(options, 
			function(e) {
				//console.log("Send_Settings --- sending");

				options.KEY_COLOR_TEXT = '0x' + GColorToHex(options.KEY_COLOR_TEXT);
				options.KEY_COLOR_INDICATOR = '0x' + GColorToHex(options.KEY_COLOR_INDICATOR);
				options.KEY_COLOR_RING_1 = '0x' + GColorToHex(options.KEY_COLOR_RING_1);
				options.KEY_COLOR_RING_2 = '0x' + GColorToHex(options.KEY_COLOR_RING_2);
				options.KEY_COLOR_BG_IN = '0x' + GColorToHex(options.KEY_COLOR_BG_IN);
				options.KEY_COLOR_BG_OUT = '0x' + GColorToHex(options.KEY_COLOR_BG_OUT);
				
				localStorage.setItem("localOptions", JSON.stringify(options));
			}, function(e) {
				console.log("Send_Settings --- failed");
			});
	} else {
		//console.log("Cancelled");
	}
});