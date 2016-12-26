export async function getDevices(cpUrl, type) {
    try {
        let response = await fetch(cpUrl + '/' + type);
        let responseJson = await response.json();
        return responseJson.devices;
    } catch(error) {
        console.error(error);
    }
}

export async function getItems(cpUrl, udn, id) {
    try {
        let response = await fetch(cpUrl + '/browse?udn=' + udn + '&id=' + id);
        let responseJson = await response.json();
        return responseJson.items;
    } catch(error) {
        console.error(error);
    }
}

export async function play(cpUrl, server, renderer, id) {
    try {
        let response = await fetch(cpUrl + '/play?serverudn=' + server + '&rendererudn=' + renderer + '&id=' + id);
        return response;
    } catch(error) {
        console.error(error);
    }
}

