import React from 'react';
import { connect } from 'react-redux'

import { ConnectedRendererButton, ConnectedServerButton } from './components/DeviceButton'
import Items from './components/Items'

import AppBar from 'material-ui/AppBar';

import { selectItem } from './actions'

const styles = {
    appbarbuttons: {
        margin: 5,
    }
};

class Doozy extends React.Component {
    render() {
        return (
            <div>
                <AppBar
                    title="Doozy"
                    iconElementRight={
                        <div style={styles.appbarbuttons}>
                            <ConnectedRendererButton/>
                            <ConnectedServerButton/>
                        </div>
                    }
                />
                <Items onSelected={item => this.props.onItemSelected(item)}
                       items={this.props.items}
                />
            </div>
        );
    }
}

const mapStateToProps = (state) => {
    return {
        controlPointUrl: state.settings.controlPointUrl,
        server: state.servers.active,
        renderer: state.renderers.active,
        items: state.items.items,
    }
}

const mapDispatchToProps = (dispatch) => {
    return {
        onItemSelected: (item) => {
            console.log('onItemSelected')
            dispatch(selectItem(item))
        }
    }
}

export default connect(mapStateToProps, mapDispatchToProps)(Doozy)
