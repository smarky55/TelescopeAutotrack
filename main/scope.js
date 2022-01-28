function panControl(id, onUpdate) {
  var range = document.getElementById(id);
  var handle = range.children[0];
  var down = false;
  var rangeWidth = range.offsetWidth;
  var rangeLeft = range.offsetWidth;
  var handleWidth = handle.offsetWidth;
  var value

  range.addEventListener("mousedown", e => {
    rangeWidth = this.offsetWidth;
    rangeLeft = this.offsetLeft;
    down = true;
    updateHandle(e);
    e.preventDefault();
    return false;
  });

  document.addEventListener("mousemove", e => {
    e.preventDefault()
    updateHandle(e);
  })

  document.addEventListener("mouseup", () => {
    resetHandle();
    down = false;
  })

  range.addEventListener("touchstart", e => {
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

  document.addEventListener("touchmove", e => {
    e.preventDefault()
    updateHandle(e.changedTouches[0]);
  })

  document.addEventListener("touchend", () => {
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

function postCommand(data) {
  return fetch("/command", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(data)
  });
}

window.onload = function () {

  // var slider = document.getElementById("slider");
  var output = document.getElementById("output");

  panControl("slider", val => {
    output.innerHTML = val

    const data = {
      command: "speed",
      value: (val * 2) - 100
    };
    postCommand(data);
  });
}

function buttonClick(element) {
  const data = { command: element.id };
  postCommand(data);
}

function updateFields() {
  var trackRateInp = document.getElementById("trackRate");
  if (trackRateInp) {
    const data = {
      command: "update",
      trackRate: trackRateInp.valueAsNumber
    };
    postCommand(data);
  }
}

function onTabClicked(tab) {
  let tabBar = tab.closest(".tabBar");
  tabBar.querySelectorAll("div").forEach((ele) => ele.classList.remove("active"));
  tab.classList.add("active");

  let tabContainer = tabBar.closest(".tabContainer")
  tabContainer.querySelectorAll(".tabContent > div").forEach((ele) => ele.style.display = "none");
  tabContainer.querySelector(".tabContent > div#" + tab.id).style.display = "block";
}

var scanInterval = null;

function pollScan() {
  postCommand({ command: "cam-list" })
    .then(resp => {
      if (!resp.ok) {
        throw new Error(resp)
      }
      return resp.json();
    })
    .then(data => {
      if (data.scanning == false) {
        clearInterval(scanInterval);
      }
      const lis = data.items.map(item => {
        const li = document.createElement("li");
        li.className = "cam-device-item";
        li.appendChild(document.createTextNode(item.name));
        li.onclick = connectToDevice.bind(null, item.address);
        return li;
      });
      document.getElementById("cam-device-list").replaceChildren(...lis);
    })
    .catch(error => {
      console.error(error);
      clearInterval(scanInterval);
    });
}

function beginScan() {
  // Send scan start command
  const data = { command: "cam-scan" };
  postCommand(data);

  scanInterval = setInterval(pollScan, 1000);
}

function connectToDevice(address) {
  const data = {
    command: "cam-connect",
    address
  }
  postCommand(data);
}