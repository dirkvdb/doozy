import React from 'react';

import AppBar from 'material-ui/AppBar';
import Popover from 'material-ui/Popover';
import RaisedButton from 'material-ui/RaisedButton';
import FlatButton from 'material-ui/FlatButton';
import Menu from 'material-ui/Menu';
import MenuItem from 'material-ui/MenuItem';
import Drawer from 'material-ui/Drawer';
import {GridList, GridTile} from 'material-ui/GridList';
import Computer from 'material-ui/svg-icons/hardware/computer';

const styles = {
  root: {
    display: 'flex',
    flexWrap: 'wrap',
    justifyContent: 'space-around',
  },
  gridList: {
    //width: 500,
    //height: 450,
    overflowY: 'auto',
  },
};

async function getDevices(type) {
    try {
        let response = await fetch('http://localhost:4444/' + type);
        let responseJson = await response.json();
        return responseJson.devices;
    } catch(error) {
        console.error(error);
    }
}

async function getItems(udn, id) {
    try {
        let response = await fetch('http://localhost:4444/browse?udn=' + udn + '&id=' + id);
        let responseJson = await response.json();
        return responseJson.items;
    } catch(error) {
        console.error(error);
    }
}

class DeviceSelectors extends React.Component {
    render() {
        return (
            <span>
                <DevicePopover name='Renderers' type='renderers'/>
                <DevicePopover name='Servers' type='servers'/>
            </span>
        );
    }
}

class DevicePopover extends React.Component {
    constructor(props) {
        super(props);

        this.state = {
            open: false,
            items: []
        };
    }

    componentDidMount() {
        var self = this;
        getDevices(this.props.type)
            .then(function(devices) {
                self.setState({items: devices});
            });
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
        this.props.onDeviceSelected(menuItem.key);
        this.setState({open: false});
    };

    render() {
        return (
            <div>
                <RaisedButton
                    onTouchTap={this.handleTouchTap}
                    label={this.props.name}
                    icon={<Computer/>}
                />
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
            </div>
        );
    }
}

class Doozy extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            open: false,
            udn: '',
            items: []
        };
    }

    handleToggle() {
        this.setState({open: !this.state.open});
    }

    handleClose() {
        this.setState({open: false});
    }

    handleServerChange(udn) {
        this.setState({items: []});
        this.setState({udn: udn});

        var self = this;
        getItems(udn, '0')
            .then(function(items) {
                self.setState({items: items});
            });
    }

    handleItemSelection(id) {
        this.setState({items: []});

        var self = this;
        getItems(this.state.udn, id)
            .then(function(items) {
                self.setState({items: items});
            });
    }

    // iconElementRight={<DevicePopover name='SERVERS'
    //                                  type='servers'
    //                                  onDeviceSelected={this.handleServerChange.bind(this)}/>}

    render() {
        return (
            <div>
                <AppBar
                    title="Doozy"
                    onLeftIconButtonTouchTap={this.handleToggle.bind(this)}
                    iconElementRight={<DeviceSelectors/>}
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
                    style={styles.gridList}
                >
                    {this.state.items.map((item) => {
                        return <GridTile key={item.id} title={item.title} onTouchTap={this.handleItemSelection.bind(this, item.id)}>
                                   <img src={item.thumbnailurl} role="presentation" />
                               </GridTile>;
                    })}
                </GridList>
            </div>
        );
    }
}

export default Doozy;
