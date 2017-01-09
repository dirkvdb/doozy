import React from 'react';
import { connect } from 'react-redux'

import { ConnectedRendererButton, ConnectedServerButton } from './components/DeviceButton'
import Items from './components/Items'

import AppBar from 'material-ui/AppBar';
import { Toolbar, ToolbarGroup } from 'material-ui/Toolbar';
import MenuItem from 'material-ui/MenuItem';
import IconMenu from 'material-ui/IconMenu';
import IconButton from 'material-ui/IconButton';
import Slider from 'material-ui/Slider';
import Expand from 'material-ui/svg-icons/navigation/expand-less';

import { selectItem } from './actions'

const styles = {
  appbarbuttons: {
    margin: 5,
  },
  toolbar: {
    position: 'fixed',
    width: '100%',
//    height: '64px',
    bottom: '0px',
  },
  toolbarImg: {
    width: '56px',
    height: '56px',
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
        <Toolbar style={styles.toolbar}>
          <ToolbarGroup firstChild={true}>
            <img style={styles.toolbarImg} src="http://192.168.1.13:9000/disk/DLNA-PNJPEG_TN-OP01-CI1-FLAGS00d00000/defaa/C/O0$1$12$15409.jpg?scale=org"></img>
          </ToolbarGroup>
          <ToolbarGroup>
            <IconButton touch={true}>
                  <Expand />
            </IconButton>
          </ToolbarGroup>
        </Toolbar>
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
