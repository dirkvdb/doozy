import { combineReducers } from 'redux'
import * as Actions from '../actions'

const initialState = {
    settings: {
        controlPointUrl: 'http://localhost:4444',
    },
    servers: {
        active: {
            name: 'Server',
            udn: ''
        },
        available: [],
    },
    renderers: {
        active: {
            name: 'Renderer',
            udn: ''
        },
        available: [],
    },
    items: {
        isFetching: false,
        items: [],
    }
}


function settings(state = initialState.settings, action) {
    switch (action.type) {
        case Actions.SET_CONTROLPOINT_URL:
            return {...state, controlPointUrl: action.url}
        default:
            return state
    }
}

function items(state = initialState.items, action) {
    switch (action.type) {
        case Actions.ITEMS_REQUEST:
            return {...state, isFetching: true}
        case Actions.ITEMS_SUCCESS:
            return {...state, isFetching: false, items: action.items}
        case Actions.ITEMS_FAILURE:
            return {...state, isFetching: false, items: []}
        default:
            return state
    }
}

function servers(state = initialState.servers, action) {
    switch (action.type) {
        case Actions.SELECT_SERVER:
            return {...state, active: {name: action.name, udn: action.udn}}
        case Actions.SERVERS_REQUEST:
            return {...state}
        case Actions.SERVERS_SUCCESS:
            return {...state, available: action.servers}
        case Actions.SERVERS_FAILURE:
            return {...state, available: []}
        default:
            return state
    }
}

function renderers(state = initialState.renderers, action) {
    switch (action.type) {
        case Actions.SELECT_RENDERER:
             return {...state, active: {name: action.name, udn: action.udn}}
        case Actions.RENDERERS_REQUEST:
            return {...state}
        case Actions.RENDERERS_SUCCESS:
            return {...state, available: action.renderers}
        case Actions.RENDERERS_FAILURE:
            return {...state, available: []}
        default:
            return state
    }
}

const doozy = combineReducers({
    settings,
    renderers,
    servers,
    items
})

export default doozy

// export default function doozy(state = initialState, action) {
//     switch (action.type) {
//         case Actions.SET_CONTROLPOINT_URL:
//             return {...state, settings: settings(state.settings, action)}

//         case Actions.ITEMS_REQUEST:
//         case Actions.ITEMS_SUCCESS:
//         case Actions.ITEMS_FAILURE:
//             return {...state, items: items(state.items, action)}

//         case Actions.SELECT_SERVER:
//         case Actions.SERVERS_REQUEST:
//         case Actions.SERVERS_SUCCESS:
//         case Actions.SERVERS_FAILURE:
//             return {...state, servers: servers(state.servers, action)}

//         case Actions.SELECT_RENDERER:
//         case Actions.RENDERERS_REQUEST:
//         case Actions.RENDERERS_SUCCESS:
//         case Actions.RENDERERS_FAILURE:
//             return {...state, renderers: renderers(state.renderers, action)}
//     default:
//         return state
//     }
// }
