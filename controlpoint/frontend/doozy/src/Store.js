import { createStore } from 'redux'
import { persistStore, autoRehydrate } from 'redux-persist'
import * as actions from './Actions'

const initialState = {
    controlPointUrl: 'http://192.168.1.10:4444',
    server: {
        name: 'Server',
        udn: ''
    },
    renderer: {
        name: 'Renderer',
        udn: ''
    }
}

function doozy(state = initialState, action) {
    switch (action.type) {
    case actions.SELECT_SERVER:
        return {...state, server: {udn: action.udn, name: action.name}}
    case actions.SELECT_RENDERER:
        return {...state, renderer: {udn: action.udn, name: action.name}}
    case actions.SET_CONTROLPOINT_URL:
        return {...state, controlPointUrl: action.url}
    default:
        return state
    }
}

const configureStore = () => {
    const store = createStore(doozy, undefined, autoRehydrate())
    persistStore(store)

    console.log(store.getState())
    store.subscribe(() =>
        console.log(store.getState())
    )

    return store
}

export default configureStore