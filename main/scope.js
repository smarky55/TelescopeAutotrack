function panControl(id, onUpdate) {
  var range = document.getElementById(id);
  var handle = range.children[0];
  var down = false;
  var rangeWidth = range.offsetWidth;
  var rangeLeft = range.offsetWidth;
  var handleWidth = handle.offsetWidth;

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

  function updateHandle(e) {
    if (down) {
      handle.style.left = Math.max(Math.min(e.pageX - rangeLeft - handleWidth / 2, rangeWidth - handleWidth), 0) + "px";
      if (typeof onUpdate == "function") onUpdate(Math.round(((e.pageX - rangeLeft) / rangeWidth) * 100));
    }
  }

  function resetHandle() {
    handle.style.left = ((rangeWidth - handleWidth) / 2) + "px";
    if (down && typeof onUpdate == "function") {
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
  const data = { button: element.id };
  fetch("/command", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(data)
  })
}