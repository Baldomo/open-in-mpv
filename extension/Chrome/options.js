import { restoreOptions, saveOptions, updateBrowserAction } from "./common.js"

const addListener = el => el.addEventListener("change", () => {
  saveOptions()
  updateBrowserAction()
})

document.addEventListener("DOMContentLoaded", restoreOptions)

Array.prototype.forEach.call(document.getElementsByTagName("input"), addListener)
Array.prototype.forEach.call(document.getElementsByTagName("select"), addListener)
