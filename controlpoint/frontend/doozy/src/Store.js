import { createStore, applyMiddleware } from 'redux'
import createLogger from 'redux-logger'
import { persistStore, autoRehydrate } from 'redux-persist'
import doozy from './reducers'

import { fetchRenderers, fetchServers, fetchItems, fetchRendererStatus } from './actions'

import thunkMiddleware from 'redux-thunk'

const configureStore = () => {
    const loggerMiddleware = createLogger()
    const store = createStore(doozy, applyMiddleware(thunkMiddleware, loggerMiddleware), autoRehydrate())
    persistStore(store, {}, () => {
        store.dispatch(fetchRenderers(store.getState().settings.controlPointUrl))
        store.dispatch(fetchServers(store.getState().settings.controlPointUrl))

        if (store.getState().servers.active.udn !== "" && store.getState().items.length === 0) {
            store.dispatch(fetchItems("0"))
        }
    })

    // setInterval(() => {
    //     fetchRendererStatus()
    // }, 5000);

    return store
}

export default configureStore