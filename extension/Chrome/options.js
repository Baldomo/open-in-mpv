import { restoreOptions, saveOptions, updateBrowserAction } from "./common.js";

function listener(el) {
    el.addEventListener("change", () => {
        saveOptions();
        updateBrowserAction();
    });
}

document.addEventListener("DOMContentLoaded", restoreOptions);

Array.prototype.forEach.call(document.getElementsByTagName("input"), listener);
Array.prototype.forEach.call(document.getElementsByTagName("select"), listener);
