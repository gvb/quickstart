// A little cross browser compatibility.
function createXMLHttpRequest() {
  // Modern browsers
  try { return new XMLHttpRequest(); } catch(e) {}
  // IE 5, 6
  try { return new ActiveXObject("Msxml2.XMLHTTP"); } catch (e) {}
  alert("XMLHttpRequest not supported");
  return null;
}


var requestTimerActive = 0;
var requestTimer;

var alertOnce = 0;

/*
 * This is the heart of the periodic poll, it requests a page every "delay"
 * milliseconds.
 */
function requestChange(page, delay) {
  var pollReq = createXMLHttpRequest();

  /*
   * Handles the AJAX (async) response.  It expects a JSON-encoded set of
   * keys and values.  The keys are DOM element IDs.  The contents of the
   * DOM elements are replaced with the values.
   */
  pollReq.onreadystatechange = function() {
    var newData;

    if (pollReq.readyState != 4)  {
      return;
    }
    if (pollReq.status != 200)  {
      // Handle error, e.g. Display error message on page
      //alert("Received error " + pollReq.status);
      return;
    }
    var serverResponse = pollReq.responseText;
    var parsed = false;

    if (typeof JSON.parse == 'function') {
      try {
        newData = JSON.parse(serverResponse);
        parsed = true;
      } catch (e) {
        if (alertOnce == 0) {
          alert("JSON.parse(" + serverResponse + ") error " + e.description);
        }
      }
    }
    // Use the simple eval() method.  The source is us, so we should be
    // able to trust it.
    if (!parsed) {
      try {
        var newData = eval('(' + serverResponse + ')');
        parsed = true;
      } catch (e) {
        if (alertOnce == 0) {
          alert("eval(" + serverResponse + ") error " + e.description);
        }
      }
    }

    if (parsed) {
      // Do a text replace for all the "key" IDs.
      for (var key in newData) {
        try {
          document.getElementById(key).innerHTML = newData[key];
        } catch (e) {
          if (alertOnce == 0) {
            alert("No key " + key);
          }
        }
      }
    }
    alertOnce = 1;
  };

  pollReq.open("GET", page, true);
  pollReq.send();

  var req = "requestChange('" + page + "'," + delay + ");";

  requestTimer = setTimeout(req, delay);
}

/*
 * Kick the polling off.
 */
function startPoll(page, delay) {
  if (!requestTimerActive) {
    requestTimerActive = 1;

    requestChange(page, delay);
  }
}

/*
 * Cancel the poll.
 */
function stopPoll() {
  clearTimeout(requestTimer);
  requestTimerActive = 0;
}

/*
 * Semi-portable: best effort try to shut down the poll when the user
 * leaves the page.
 */
window.onbeforeunload = function() {
  stopPoll();
}

/*
 * Report a button click.
 */
function btnReport(which) {
  var btnReportMsg = createXMLHttpRequest();
  btnReportMsg.open("GET", "/button?" + which, true);
  btnReportMsg.send();
}

