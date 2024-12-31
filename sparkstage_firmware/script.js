"use strict";

const modes = [
    "auto",
    "singleColor",
    "singleBlink",
    "singleWave",
];


const parseColorHexString = (color) => {
    let c = { "r": 0, "g": 0, "b": 0 };
    const r_string = color.substring(0, 2);
    const g_string = color.substring(2, 4);
    const b_string = color.substring(4, 6);
    c["r"] = parseInt(r_string, 16);
    c["g"] = parseInt(g_string, 16);
    c["b"] = parseInt(b_string, 16);

    return c;
};


const parseConfigString = (configString) => {
    let config = { "mode": 0, "color": null };
    const mode_string = configString.substring("MODE:".length, "MODE:".length + 1);
    config.mode = parseInt(mode_string, 16);
    const color_start_index = configString.indexOf("COLOR:") + "COLOR:".length;
    const color_string = configString.substring(color_start_index, color_start_index + 6);
    config.color = parseColorHexString(color_string);

    return config;
};

const generateColorString = (color) => {
    let s = "";

    if (color["r"] < 0x10) {
        s += "0";
    }
    s += color["r"].toString(16);

    if (color["g"] < 0x10) {
        s += "0";
    }
    s += color["g"].toString(16);

    if (color["b"] < 0x10) {
        s += "0";
    }
    s += color["b"].toString(16);

    return s;
};

const putConfig = async (mode, color, save = false) => {
    // mode: int
    // color: { "r": int, "g": int, "b": int }

    // let modeInt = modes.findIndex((e) => e == mode);
    // if (modeInt < 0) {
    //     modeInt = 0;
    // }
    const modeInt = mode;

    let configString = `MODE:${modeInt},COLOR:${generateColorString(color)}`;
    configString = encodeURIComponent(configString);

    let url;
    if (save) {
        url = "/config_save";
    } else {
        url = "/config";
    }
    const request = new Request(url, {
        method: "POST",
        body: "config=" + configString,
        headers: new Headers({
            "Content-Type": "application/x-www-form-urlencoded"
        })
    });
    await fetch(request);
};


const putWiFiSTAConfig = (ssid, psk) => {
    const postData = new FormData();
    postData.append("s", ssid);
    postData.append("p", psk);
    const url = "/config_wifi";
    const req = new Request(url, {
        method: "POST",
        body: postData
    });
    fetch(req)
        .then((res) => { if (res.ok) {
            alert("OK. Please reboot manually.");
        }})
        .catch((err) => { alert("Failed:", err); });
};


const requestResetConfig = async () => {

};


document.addEventListener("DOMContentLoaded", () => {
    const saveButton = document.getElementById("configSaveButton");
    const redSlider = document.getElementById('red');
    const greenSlider = document.getElementById('green');
    const blueSlider = document.getElementById('blue');
    const redValue = document.getElementById('redValue');
    const greenValue = document.getElementById('greenValue');
    const blueValue = document.getElementById('blueValue');
    const colorDisplay = document.getElementById('colorDisplay');
    const colorCode = document.getElementById('colorCode');
    const modeRadioElements = document.querySelectorAll('input[name="mode"]');
    const wifiSSIDText = document.querySelector("#wifiSSIDText");
    const wifiPSKText = document.querySelector("#wifiPSKText");
    const wifiSTAConnectButton = document.querySelector("#wifiSTAConnectButton");

    const getSelectedMode = (defaultValue) => {
        for (let i = 0; i < modeRadioElements.length; i++) {
            const cur = modeRadioElements[i];
            if (cur.checked) {
                // return modeRadioElements[i].value;
                let modeInt = modes.findIndex((e) => e == cur.value);
                if (modeInt >= 0) {
                    return modeInt;
                } else {
                    return defaultValue;
                }
            }
        }
        return defaultValue;
    };

    const applyConfigToUI = (configString) => {
        
        // Parse "MODE:0,COLOR:0123ab"
        const modeValue = parseInt(configString.substring(5, 6));
        const colorString = configString.substring(13, 19);

        // Update mode radio
        for (let i = 0; i < modeRadioElements.length; i++) {
            if (modeRadioElements[i].value == modes[modeValue]) {
                modeRadioElements[i].checked = true;
            } else {
                modeRadioElements[i].checked = false;
            }
        }

        // Update color slider
        const red = parseInt(colorString.substring(0, 2), 16);
        const green = parseInt(colorString.substring(2, 4), 16);
        const blue = parseInt(colorString.substring(4, 6), 16);
        redSlider.value = red;
        greenSlider.value = green;
        blueSlider.value = blue;
        redValue.textContent = red;
        greenValue.textContent = green;
        blueValue.textContent = blue;
    };

    const getConfig = async () => {
        // configString = "MODE:1,COLOR:0123ab";
        fetch("/config")
            .then((res) => { if (res.ok) { return res.text(); } })
            .then((text) => { applyConfigToUI(text); })
            .catch((err) => alert(err) );

        // applyConfigToUI("MODE:1,COLOR:0123ab");
    };
    
    // Update the display when sliders change
    const updateColor = async (save = false) => {
        const red = redSlider.value;
        const green = greenSlider.value;
        const blue = blueSlider.value;

        // Update text values
        redValue.textContent = red;
        greenValue.textContent = green;
        blueValue.textContent = blue;

        // Update color display and color code
        const rgbColor = `rgb(${red}, ${green}, ${blue})`;
        colorDisplay.style.backgroundColor = rgbColor;
        colorCode.textContent = rgbColor;

        // Update background color of the page
        document.body.style.backgroundColor = rgbColor;

        const modeInt = getSelectedMode(1);

        putConfig(modeInt, { "r": parseInt(red), "g": parseInt(green), "b": parseInt(blue) }, save)
            .then(() => {
                // alert("OK.");
                }
            )
            .catch((err) => alert(err) );

    };

    saveButton.addEventListener("click", () => {
        updateColor(true)
        .then(() => { alert("OK. Saved."); })
        .catch((err) => { alert(err); });
    });

    // Add event listener to radio options
    for (let i = 0; i < modeRadioElements.length; i++) {
        modeRadioElements[i].addEventListener("change", () => {updateColor(false); });
    }

    // Add event listeners to the sliders
    redSlider.addEventListener('input', () => { updateColor(false); });
    greenSlider.addEventListener('input', () => { updateColor(false); });
    blueSlider.addEventListener('input', () => { updateColor(false); });

    wifiSTAConnectButton.addEventListener("click", () => {
        const ssid = wifiSSIDText.value;
        const psk = wifiPSKText.value;

        putWiFiSTAConfig(ssid, psk);
    });

    getConfig()
        .then(() => {

            updateColor(false)
            .then(() => {})
            .catch((err) => alert(err) );

        })
        .catch((err) => alert(err) );

});
