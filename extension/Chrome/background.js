import { getOptions, openInMPV, updateBrowserAction } from "./common.js";

updateBrowserAction();

const dict = {
  page: "pageUrl",
  link: "linkUrl",
  video: "srcUrl",
  audio: "srcUrl",
};

Object.keys(dict).forEach((item) => {
  chrome.contextMenus.create({
    title: `Open this ${item} in mpv`,
    id: `openinmpv_${item}`,
    contexts: [item],
  });
});

chrome.contextMenus.onClicked.addListener(function (info, tab) {
  if (info.menuItemId.startsWith("openinimpv")) {
    const key = info.menuItemId.split("_")[1];
    const url = info[dict[key]];
    if (url) {
      getOptions((options) => {
        openInMPV(tab.id, url, options);
      })
    }
  }
});

chrome.action.onClicked.addListener(() => {
  // get active window
  chrome.tabs.query({ currentWindow: true, active: true }, (tabs) => {
    if (tabs.length === 0) {
      return;
    }
    // TODO: filter url
    const tab = tabs[0];
    if (tab.id === chrome.tabs.TAB_ID_NONE) {
      return;
    }
    getOptions((options) => {
      openInMPV(tab.id, tab.url, {
        mode: options.iconActionOption,
        ...options,
      });
    });
  });
});

