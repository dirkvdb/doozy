const SELECT_RENDERER = 'SELECT_RENDERER'
const SELECT_SERVER = 'SELECT_SERVER'
const SET_CONTROLPOINT_URL = 'SET_CONTROLPOINT_URL'

export function selectRenderer(device) {
    return {
        type: SELECT_RENDERER,
        udn: device.udn,
        name: device.name
    }
}

export function selectServer(device) {
    return {
        type: SELECT_SERVER,
        udn: device.udn,
        name: device.name
    }
}

export function setControlPointUrl(url) {
    return {
        type: SET_CONTROLPOINT_URL,
        url: url
    }
}
