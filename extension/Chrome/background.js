import { getOptions, openInMPV, updateBrowserAction } from "./common.js";

updateBrowserAction();

[["page", "pageUrl"], ["link", "linkUrl"], ["video", "srcUrl"], ["audio", "srcUrl"]].forEach(([item, linkType]) => {
  chrome.contextMenus.create({
    title: `Open this ${item} in mpv`,
    id: `open${item}inmpv`,
    contexts: [item],
    onclick: (info, tab) => {
      getOptions((options) => {
        console.log("Got options: ", options);
        openInMPV(tab.id, info[linkType], {
          mode: options.iconActionOption,
          ...options,
        });
      });
    },
  });
});
