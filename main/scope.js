function panControl(id, onUpdate) {
  var range = document.getElementById(id);
  var handle = range.children[0];
  var down = false;
  var rangeWidth = range.offsetWidth;
  var rangeLeft = range.offsetWidth;
  var handleWidth = handle.offsetWidth;
  var value

  range.addEventListener("mousedown", function (e) {
    rangeWidth = this.offsetWidth;
    rangeLeft = this.offsetLeft;
    down = true;
    updateHandle(e);
    e.preventDefault();
    return false;
  });

  document.addEventListener("mousemove", function (e) {
    e.preventDefault()
    updateHandle(e);
  })

  document.addEventListener("mouseup", function (e) {
    resetHandle();
    down = false;
  })

  range.addEventListener("touchstart", function (e) {
    rangeWidth = this.offsetWidth;
    rangeLeft = this.offsetLeft;
    let rangeHeight = this.offsetHeight;
    let rangeTop = this.offsetTop;
    if (rangeLeft < e.changedTouches[0].pageX && rangeLeft + rangeWidth > e.changedTouches[0].pageX
      && rangeTop < e.changedTouches[0].pageY && rangeTop + rangeHeight > e.changedTouches[0].pageY) {
      down = true;
      updateHandle(e.changedTouches[0]);
      e.preventDefault();
    }
    return false;
  });

  document.addEventListener("touchmove", function (e) {
    e.preventDefault()
    updateHandle(e.changedTouches[0]);
  })

  document.addEventListener("touchend", function () {
    resetHandle();
    down = false;
  })

  function updateHandle(e) {
    if (down) {
      handle.style.left = Math.max(Math.min(e.pageX - rangeLeft - handleWidth / 2, rangeWidth - handleWidth), 0) + "px";
      let newValue = Math.round(((e.pageX - rangeLeft) / rangeWidth) * 100);
      if (typeof onUpdate == "function" && newValue != value) {
        value = newValue;
        onUpdate(value);
      }
    }
  }

  function resetHandle() {
    handle.style.left = ((rangeWidth - handleWidth) / 2) + "px";
    if (down && typeof onUpdate == "function") {
      value = 50;
      onUpdate(50);
    }
  }

  resetHandle();
}

window.onload = function () {

  // var slider = document.getElementById("slider");
  var output = document.getElementById("output");

  panControl("slider", function (val) {
    output.innerHTML = val

    const data = {
      command: "speed",
      value: (val * 2) - 100
    };
    fetch("/command", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(data)
    });
  });
}

function buttonClick(element) {
  const data = { command: element.id };
  fetch("/command", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(data)
  });
}

function updateFields() {
  var trackRateInp = document.getElementById("trackRate");
  if (trackRateInp) {
    const data = {
      command: "update",
      trackRate: trackRateInp.valueAsNumber
    };
    fetch("/command", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(data)
    });
  }
}