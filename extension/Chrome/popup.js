import { openInMPV, getOptions, getActiveTab } from "./common.js"

Array.prototype.forEach.call(document.getElementsByClassName("menu-item"), item => {
  const mode = item.id.split("-")[1]
  item.addEventListener("click", () => {
    getOptions(options => {
      getActiveTab((tab) => {
        if (tab) {
          openInMPV(tab.id, tab.url, {
            mode,
            newWindow: mode === "newWindow",
            ...options,
          })
        }
      })
    })
  })
})
