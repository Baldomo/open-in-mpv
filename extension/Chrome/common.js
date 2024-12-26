class Option {
  constructor(name, type, defaultValue) {
    this.name = name
    this.type = type
    this.defaultValue = defaultValue
  }

  setValue(value) {
    switch (this.type) {
      case "radio":
        Array.prototype.forEach.call(document.getElementsByName(this.name), (el) => {
          el.checked = el.value === value
        })
        break
      case "checkbox":
        document.getElementsByName(this.name).forEach(el => el.checked = value)
        break
      case "select":
        document.getElementsByName(this.name).forEach(el => el.value = value)
        break
      case "text":
        document.getElementsByName(this.name).forEach(el => el.value = value)
        break
    }
  }

  getValue() {
    switch (this.type) {
      case "radio":
        return document.querySelector(`input[name="${this.name}"]:checked`).value
      case "checkbox":
        return document.querySelector(`input[name="${this.name}"]`).checked
      case "select":
        return document.querySelector(`select[name="${this.name}"]`).value
      case "text":
        return document.querySelector(`input[name="${this.name}"]`).value
    }
  }
}

const options = [
  new Option("iconAction", "radio", "clickOnly"),
  new Option("iconActionOption", "radio", "direct"),
  new Option("mpvPlayer", "select", "mpv"),
  new Option("useCustomFlags", "checkbox", false),
  new Option("customFlags", "text", "")
]

export function getOptions(callback) {
  const defaults = {}
  options.forEach(item => {
    defaults[item.name] = item.defaultValue
  })
  chrome.storage.sync.get(defaults, callback)
}

export function saveOptions() {
  const saveDict = {}
  options.forEach(item => {
    saveDict[item.name] = item.getValue()
  })
  chrome.storage.sync.set(saveDict)
}

export function restoreOptions() {
  getOptions((items) => {
    options.forEach(option => {
      option.setValue(items[option.name])
    })
  })
}

function openLink(baseURL, params = []) {
  const link = document.createElement('a')
  link.href = `${baseURL}${params.join("&")}`
  document.body.appendChild(link)
  link.click()
}

export function openInMPV(tabId, url, options = {}) {
  const baseURL = `mpv:///open?`

  // Encode video URL
  const params = [`url=${encodeURIComponent(url)}`]

  // Add playback options
  switch (options.mode) {
    case "fullScreen":
      params.push("full_screen=1"); break
    case "pip":
      params.push("pip=1"); break
    case "enqueue":
      params.push("enqueue=1"); break
  }

  // Add new window option
  if (options.newWindow) {
    params.push("new_window=1")
  }

  // Add alternative player and user-defined custom flags
  params.push(`player=${options.mpvPlayer}`)
  if (options.useCustomFlags && options.customFlags !== "")
    params.push(`flags=${encodeURIComponent(options.customFlags)}`)

  console.log("Opening URL:", { baseURL, params })
  chrome.scripting.executeScript({
    target: { tabId },
    func: openLink,
    args: [baseURL, params]
  })
}

export function updateBrowserAction() {
  getOptions((options) => {
    if (options.iconAction === "clickOnly") {
      chrome.action.setPopup({ popup: "" });
    } else {
      chrome.action.setPopup({ popup: "popup.html" });
    }
  });
}
