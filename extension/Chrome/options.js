import { restoreOptions, saveOptions, updateBrowserAction } from "./common.js"
import { shortcutListener } from "./keyboard.js"

const addListener = el => el.addEventListener("change", () => {
    saveOptions()
    updateBrowserAction()
    shortcutListener()
})

document.addEventListener("DOMContentLoaded", restoreOptions)

Array.prototype.forEach.call(document.getElementsByTagName("input"), addListener)
Array.prototype.forEach.call(document.getElementsByTagName("select"), addListener)
