import { getOptions, getActiveTab, openInMPV } from "./common.js";

function hasCommandListener(listener) {
  try {
    return chrome.commands.onCommand.hasListener(listener)
  } catch (e) {
    return false
  }
}

function handleCommand(command) {
  if (command === "open-in-mpv-shortcut") {
    getOptions(options => {
      getActiveTab((tab) => {
        if (tab) {
          openInMPV(tab.id, tab.url, {
            mode: options.iconActionOption,
            ...options,
          })
        }
      })
    })
  }
}

export function shortcutListener() {
  getOptions((options) => {
    const shortcutsEnabled = options.useShortcut
    const listenerExists = hasCommandListener(handleCommand)

    if (shortcutsEnabled && !listenerExists) {
      chrome.commands.onCommand.addListener(handleCommand)
    } else if (!shortcutsEnabled && listenerExists) {
      chrome.commands.onCommand.removeListener(handleCommand)
    }
  })
}
