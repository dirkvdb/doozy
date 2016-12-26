import React from 'react';
import { connect } from 'react-redux'

import * as cp from './ControlPoint'
import { ConnectedRendererButton, ConnectedServerButton } from './DeviceButton'

import AppBar from 'material-ui/AppBar';
import MenuItem from 'material-ui/MenuItem';
import Drawer from 'material-ui/Drawer';
import {GridList, GridTile} from 'material-ui/GridList';

const styles = {
    appbarbuttons: {
        margin: 5,
    }
};

class Doozy extends React.Component {
    constructor(props) {
        super(props);

        this.state = {
            open: false,
            gridcols: 2,
            items: []
        };
    }

    componentDidMount() {
        console.log('Doozy::componentDidMount')
        console.log(this.props.server)

        if (typeof this.props.server !== "undefined" && this.props.server.udn !== "") {
            var self = this;
            cp.getItems(this.props.controlPointUrl, this.props.server.udn, '0')
                .then(function(items) {
                    self.setState({items: items});
                });
        }
    }

    handleToggle() {
        this.setState({open: !this.state.open});
    }

    handleClose() {
        this.setState({open: false});
    }

    handleServerChange(device) {
        console.log('Server selected: ' + device.udn + ' '+ JSON.stringify(device));

        this.setState({items: []});
        this.setState({server: device});

        var self = this;
        cp.getItems(this.props.controlPointUrl, device.udn, '0')
            .then(function(items) {
                self.setState({items: items});
            });
    }

    handleRendererChange(device) {
        console.log('Renderer selected: ' + device.udn);
        this.setState({renderer: device});
    }

    handleItemSelection(item) {
        console.log('Item selected: ' + item.title + ' [' + item.class + ']');

        if (item.class === 'object.container.album.musicAlbum') {
            if (this.state.renderer) {
                cp.play(this.props.controlPointUrl, this.props.server.udn, this.props.renderer.udn, item.id);
            }

            return;
        }

        this.setState({items: []});

        var self = this;
        cp.getItems(this.props.controlPointUrl, this.state.server.udn, item.id)
            .then(function(items) {
                self.setState({items: items});
            });
    }

    render() {
        return (
            <div>
                <AppBar
                    title="Doozy"
                    onLeftIconButtonTouchTap={this.handleToggle.bind(this)}
                    iconElementRight={
                        <div style={styles.appbarbuttons}>
                            <ConnectedRendererButton/>
                            <ConnectedServerButton/>
                        </div>
                    }
                />
                <Drawer
                    docked={false}
                    open={this.state.open}
                    onRequestChange={(open) => this.setState({open})}
                >
                    <MenuItem onTouchTap={this.handleClose.bind(this)}>Menu Item 1</MenuItem>
                    <MenuItem onTouchTap={this.handleClose.bind(this)}>Menu Item 2</MenuItem>
                    <MenuItem onTouchTap={this.handleClose.bind(this)}>Menu Item 3</MenuItem>
                </Drawer>
                <GridList
                    cellHeight={180}
                    cols={this.state.gridcols}
                >
                    {this.state.items.map((item) => {
                        return <GridTile key={item.id} title={item.title} onTouchTap={this.handleItemSelection.bind(this, item)}>
                                   <img src={item.thumbnailurl} role="presentation" />
                               </GridTile>;
                    })}
                </GridList>
            </div>
        );
    }
}

const mapStateToProps = (state) => {
    return {controlPointUrl: state.controlPointUrl, server: state.server, renderer: state.renderer}
}

const mapDispatchToProps = (dispatch) => {
    return {
        onServerSelected: (device) => {
            //dispatch(selectServer(device))
            console.log('Server selected')
        }
    }
}

const ConnectedDoozy = connect(mapStateToProps, mapDispatchToProps)(Doozy)

export default ConnectedDoozy;
