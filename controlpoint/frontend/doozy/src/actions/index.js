// User initiated actions
export const SELECT_RENDERER = 'SELECT_RENDERER'
export const SELECT_SERVER = 'SELECT_SERVER'
export const SELECT_ITEM = 'SELECT_ITEM'
export const SET_CONTROLPOINT_URL = 'SET_CONTROLPOINT_URL'
export const REFRESH_SERVERS = 'REFRESH_SERVERS'
export const REFRESH_RENDERERS = 'REFRESH_RENDERERS'

export function selectRenderer(device) {
    return {
        type: SELECT_RENDERER,
        udn: device.udn,
        name: device.name,
    }
}

function serverChanged(device) {
    return {
        type: SELECT_SERVER,
        udn: device.udn,
        name: device.name,
    }
}

export function setControlPointUrl(url) {
    return {
        type: SET_CONTROLPOINT_URL,
        url: url,
    }
}

export function refreshServers() {
    return {
        type: REFRESH_SERVERS,
    }
}

export function refreshRenderers() {
    return {
        type: REFRESH_RENDERERS,
    }
}


export const SERVERS_REQUEST = 'SERVERS_REQUEST'
export const SERVERS_SUCCESS = 'SERVERS_SUCCESS'
export const SERVERS_FAILURE = 'SERVERS_FAILURE'

export function requestServers() {
    return {
        type: SERVERS_REQUEST,
    }
}

export function receiveServers(devices) {
    return {
        type: SERVERS_SUCCESS,
        servers: devices,
    }
}

export function requestServersFailed() {
    return {
        type: SERVERS_FAILURE,
    }
}

export const RENDERERS_REQUEST = 'RENDERERS_REQUEST'
export const RENDERERS_SUCCESS = 'RENDERERS_SUCCESS'
export const RENDERERS_FAILURE = 'RENDERERS_FAILURE'

export function requestRenderers() {
    return {
        type: RENDERERS_REQUEST,
    }
}

export function receiveRenderers(devices) {
    return {
        type: RENDERERS_SUCCESS,
        renderers: devices,
    }
}

export function requestRenderersFailed() {
    return {
        type: RENDERERS_FAILURE,
    }
}

export const ITEMS_REQUEST = 'ITEMS_REQUEST'
export const ITEMS_SUCCESS = 'ITEMS_SUCCESS'
export const ITEMS_FAILURE = 'ITEMS_FAILURE'

export function requestItems() {
    return {
        type: ITEMS_REQUEST,
    }
}

export function receiveItems(items) {
    return {
        type: ITEMS_SUCCESS,
        items: items,
    }
}

export function requestItemsFailed(error) {
    return {
        type: ITEMS_FAILURE,
        error: error
    }
}

export const RENDERER_STATUS_REQUEST = 'RENDERER_STATUS_REQUEST'
export const RENDERER_STATUS_SUCCESS = 'RENDERER_STATUS_SUCCESS'
export const RENDERER_STATUS_FAILURE = 'RENDERER_STATUS_FAILURE'

export function requestRendererStatus() {
    return {
        type: RENDERER_STATUS_REQUEST,
    }
}

export function receiveRendererStatus(status) {
    return {
        type: RENDERER_STATUS_SUCCESS,
        items: status,
    }
}

export function requestRendererStatusFailed(error) {
    return {
        type: RENDERER_STATUS_FAILURE,
        error: error
    }
}

export const PLAY_REQUEST = 'PLAY_REQUEST'
export const PLAY_SUCCESS = 'PLAY_SUCCESS'
export const PLAY_FAILURE = 'PLAY_FAILURE'

export function playRequested(id) {
    return {
        type: PLAY_REQUEST,
        id: id,
    }
}

export function playSucceeded() {
    return {
        type: PLAY_SUCCESS,
    }
}

export function playFailed(error) {
    return {
        type: PLAY_FAILURE,
        error: error,
    }
}

export function fetchServers(cpUrl) {
    return function (dispatch) {
        dispatch(requestServers())

        return fetch(cpUrl + '/servers')
            .then(response => response.json())
            .then(json => dispatch(receiveServers(json.devices)))
            .catch(error => dispatch(requestServersFailed()))
      }
}

export function fetchRenderers(cpUrl) {
    return function (dispatch) {
        dispatch(requestRenderers())

        return fetch(cpUrl + '/renderers')
            .then(response => response.json())
            .then(json => dispatch(receiveRenderers(json.devices)))
            .catch(error => dispatch(requestRenderersFailed()))
      }
}

export function fetchRendererStatus() {
    return function (dispatch, getState) {
        dispatch(requestRendererStatus())

        const cpUrl = getState().settings.controlPointUrl
        const renderer = getState().renderers.active

        return fetch(cpUrl + '/rendererstatus?udn=' + renderer.udn)
            .then(response => response.json())
            .then(json => dispatch(receiveRendererStatus(json)))
            .catch(error => dispatch(requestRendererStatusFailed()))
      }
}

function playItem(id) {
    return (dispatch, getState) => {
        dispatch(playRequested(id))

        const renderer = getState().renderers.active
        const server = getState().servers.active
        const cpUrl = getState().settings.controlPointUrl

        return fetch(cpUrl + '/play?serverudn=' + server.udn + '&rendererudn=' + renderer.udn + '&id=' + id)
            .then(response => dispatch(playSucceeded()))
            .catch(error => dispatch(playFailed(error)))
    }
}

export function fetchItems(id) {
    return (dispatch, getState) => {
        dispatch(requestItems())

        const server = getState().servers.active
        const cpUrl = getState().settings.controlPointUrl

        return fetch(cpUrl + '/browse?udn=' + server.udn + '&id=' + id)
            .then(response => response.json())
            .then(json => dispatch(receiveItems(json.items)))
            .catch(error => dispatch(requestItemsFailed(error)))
    }
}

export function selectItem(item) {
    return function (dispatch, getState) {
        if (item.class === 'object.container.album.musicAlbum') {
            return dispatch(playItem(item.id))
        } else {
            return dispatch(fetchItems(item.id));
        }
    }
}

export function selectServer(device) {
    return (dispatch, getState) => {
        dispatch(serverChanged(device))
        if (device.udn !== "") {
            return dispatch(fetchItems("0"))
        } else {
            return Promise.resolve()
        }
    }
}

