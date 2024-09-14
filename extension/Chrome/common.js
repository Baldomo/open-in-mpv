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

const _options = [
  new Option("iconAction", "radio", "clickOnly"),
  new Option("iconActionOption", "radio", "direct"),
  new Option("mpvPlayer", "select", "mpv"),
  new Option("useCustomFlags", "checkbox", false),
  new Option("customFlags", "text", ""),
  new Option("useShortcut", "checkbox", false),
  new Option("shortcutKey", "text", "")
]

function saveShortcut(txt) {
  // firefox only - chrome doesn't support updating shortcuts dynamically through text box
  if (navigator.userAgent.includes("Firefox")) {
    browser.commands.update({
      name: "open-in-mpv-shortcut",
      shortcut: txt.shortcutKey
    });
  }
}

function restoreShortcut() {
  getOptions((items) => {
    chrome.commands.getAll((commands) => {
      const cmd = commands.find(cmd => cmd.name === "open-in-mpv-shortcut")
      _options.find(option => option.name === "shortcutKey").setValue(cmd.shortcut)
    })
  })
}

export function getOptions(callback) {
  const getDict = {}
  _options.forEach(item => {
    getDict[item.name] = item.defaultValue
  })
  chrome.storage.sync.get(getDict, callback)
}

export function saveOptions() {
  const saveDict = {}
  _options.forEach(item => {
    saveDict[item.name] = item.getValue()
  })
  chrome.storage.sync.set(saveDict)
  saveShortcut(saveDict.shortcutKey)
}

export function restoreOptions() {
  getOptions((items) => {
    _options.forEach(option => {
      option.setValue(items[option.name])
    })
  })
  restoreShortcut()
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

  const code = `
    var link = document.createElement('a')
    link.href='${baseURL}${params.join("&")}'
    document.body.appendChild(link)
    link.click()`
  console.log(code)
  chrome.tabs.executeScript(tabId, { code })
}

export function getActiveTab(cb) {
  chrome.tabs.query({ currentWindow: true, active: true }, (tabs) => {
    const tab = tabs[0]
    if (!tab || tab.id === chrome.tabs.TAB_ID_NONE) {
      cb(null)
    } else {
      cb(tab)
    }
  })
}

export function updateBrowserAction() {
  getOptions(options => {
    if (options.iconAction === "clickOnly") {
      chrome.browserAction.setPopup({ popup: "" })
      chrome.browserAction.onClicked.addListener(() => {
        getActiveTab((tab) => {
          if (tab) {
            openInMPV(tab.id, tab.url, {
              mode: options.iconActionOption,
              ...options,
            })
          }
        })
      })

      return
    }

    chrome.browserAction.setPopup({ popup: "popup.html" })
  })
}
