import React from 'react';

import AppBar from 'material-ui/AppBar';
import Popover from 'material-ui/Popover';
import RaisedButton from 'material-ui/RaisedButton';
import Menu from 'material-ui/Menu';
import MenuItem from 'material-ui/MenuItem';
import Drawer from 'material-ui/Drawer';
import {GridList, GridTile} from 'material-ui/GridList';

import Computer from 'material-ui/svg-icons/hardware/computer';
import Speaker from 'material-ui/svg-icons/hardware/speaker';

const styles = {
    appbarbuttons: {
        margin: 5,
    },
    appbarbutton: {
        marginRight: 8,
    }
};

var serverip = '192.168.1.10';

async function getDevices(type) {
    try {
        let response = await fetch('http://' + serverip + ':4444/' + type);
        let responseJson = await response.json();
        return responseJson.devices;
    } catch(error) {
        console.error(error);
    }
}

async function getItems(udn, id) {
    try {
        let response = await fetch('http://' + serverip + ':4444/browse?udn=' + udn + '&id=' + id);
        let responseJson = await response.json();
        return responseJson.items;
    } catch(error) {
        console.error(error);
    }
}

async function play(server, renderer, id) {
    try {
        let response = await fetch('http://' + serverip + ':4444/play?serverudn=' + server + '&rendererudn=' + renderer + '&id=' + id);
        return response;
    } catch(error) {
        console.error(error);
    }
}

class DevicePopover extends React.Component {
    constructor(props) {
        super(props);

        this.state = {
            open: false,
            buttonText: '',
            items: []
        };
    }

    componentDidMount() {
        this.handleResize()
        window.addEventListener("resize", this.handleResize.bind(this));

        var self = this;
        getDevices(this.props.type)
            .then(function(devices) {
                self.setState({items: devices});
            });
    }

    handleResize () {
        var width = window.innerWidth;
        this.setState({buttonText: width > 500 ? this.props.name : ''});
    }

    handleTouchTap = (event) => {
        // This prevents ghost click.
        event.preventDefault();

        this.setState({
            open: true,
            anchorEl: event.currentTarget
        });
    };

    handleRequestClose = () => {
        this.setState({open: false});
    };

    handleDeviceSelected = (event, menuItem, index) => {
        this.props.onDeviceSelected({udn: menuItem.key, name: menuItem.props.primaryText});
        this.setState({open: false});
    };

    render() {
        return (
            <RaisedButton style={styles.appbarbutton}
                onTouchTap={this.handleTouchTap}
                label={this.state.buttonText}
                icon={this.props.type === 'servers' ? <Computer/> : <Speaker/>}
            >
                <Popover
                   open={this.state.open}
                   anchorEl={this.state.anchorEl}
                   anchorOrigin={{horizontal: 'left', vertical: 'bottom'}}
                   targetOrigin={{horizontal: 'left', vertical: 'top'}}
                   onRequestClose={this.handleRequestClose}
                >
                   <Menu onItemTouchTap={this.handleDeviceSelected}>
                       {this.state.items.map((item) => {
                           return <MenuItem key={item.udn} primaryText={item.name} />
                       })}
                   </Menu>
                </Popover>
            </RaisedButton>
        );
    }
}

class Doozy extends React.Component {
    constructor(props) {
        super(props);

        try {
            var server = JSON.parse(localStorage.getItem('server') || {name: 'Servers'});
            var renderer = JSON.parse(localStorage.getItem('renderer') || {name: 'Renderers'});
        } catch (error) {
            server = {name: 'Servers'}
            renderer = {name: 'Renderers'}
        }

        console.log(server);
        console.log(renderer);

        this.state = {
            open: false,
            server: server,
            renderer: renderer,
            gridcols: 2,
            items: []
        };
    }

    componentDidMount() {
        if (typeof this.state.server.udn !== "undefined") {
            var self = this;
            getItems(this.state.server.udn, '0')
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
        localStorage.setItem('server', JSON.stringify(device));

        this.setState({items: []});
        this.setState({server: device});

        var self = this;
        getItems(device.udn, '0')
            .then(function(items) {
                self.setState({items: items});
            });
    }

    handleRendererChange(device) {
        console.log('Renderer selected: ' + device.udn);
        localStorage.setItem('renderer', JSON.stringify(device));
        this.setState({renderer: device});
    }

    handleItemSelection(item) {
        console.log('Item selected: ' + item.title + ' [' + item.class + ']');

        if (item.class === 'object.container.album.musicAlbum') {
            if (this.state.renderer) {
                play(this.state.server.udn, this.state.renderer.udn, item.id);
            }

            return;
        }

        this.setState({items: []});

        var self = this;
        getItems(this.state.server.udn, item.id)
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
                            <DevicePopover name={this.state.renderer.name} type='renderers' onDeviceSelected={this.handleRendererChange.bind(this)}/>
                            <DevicePopover name={this.state.server.name} type='servers' onDeviceSelected={this.handleServerChange.bind(this)}/>
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

export default Doozy;
