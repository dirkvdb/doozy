import React from 'react';
import { connect } from 'react-redux'

import * as actions from '../actions'

import Popover from 'material-ui/Popover';
import RaisedButton from 'material-ui/RaisedButton';
import Menu from 'material-ui/Menu';
import MenuItem from 'material-ui/MenuItem';

import Computer from 'material-ui/svg-icons/hardware/computer';
import Speaker from 'material-ui/svg-icons/hardware/speaker';

const styles = {
    appbarbutton: {
        marginRight: 8,
    }
};

class DeviceButton extends React.Component {
    constructor(props) {
        super(props);

        this.state = {
            open: false
        };
    }

    componentDidMount() {
        this.handleResize()
        window.addEventListener("resize", this.handleResize.bind(this));
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

    handleDeviceSelected(event, menuItem, index) {
        this.setState({open: false});
    }

    render() {
        return (
            <RaisedButton style={styles.appbarbutton}
                onTouchTap={this.handleTouchTap}
                label={this.props.name}
                icon={this.state.icon}
            >
                <Popover
                   open={this.state.open}
                   anchorEl={this.state.anchorEl}
                   anchorOrigin={{horizontal: 'left', vertical: 'bottom'}}
                   targetOrigin={{horizontal: 'left', vertical: 'top'}}
                   onRequestClose={this.handleRequestClose}
                >
                   <Menu onItemTouchTap={this.handleDeviceSelected.bind(this)}>
                       {this.props.devices.map((dev) => {
                           return <MenuItem key={dev.udn} primaryText={dev.name} />
                       })}
                   </Menu>
                </Popover>
            </RaisedButton>
        );
    }
}

class ServerButton extends DeviceButton {
    constructor(props) {
        super(props)
        this.state.icon = <Computer/>
    }

    handleDeviceSelected(event, menuItem, index) {
        super.handleDeviceSelected(event, menuItem, index)
        this.props.onServerSelected({udn: menuItem.key, name: menuItem.props.primaryText})
    }
}

class RendererButton extends DeviceButton {
    constructor(props) {
        super(props)
        this.state.icon = <Speaker/>
    }

    handleDeviceSelected(event, menuItem, index) {
        super.handleDeviceSelected(event, menuItem, index)
        this.props.onRendererSelected({udn: menuItem.key, name: menuItem.props.primaryText})
    }
}

const mapStateToProps = (state) => {
    return {
        name: state.servers.active.name,
        devices: state.servers.available
    }
}

const mapRStateToProps = (state) => {
    return {
        name: state.renderers.active.name,
        devices: state.renderers.available,
    }
}

const mapDispatchToProps = (dispatch) => {
    return {
        onServerSelected: (device) => {
            dispatch(actions.selectServer(device))
        },
        onRendererSelected: (device) => {
            dispatch(actions.selectRenderer(device))
        }
    }
}

export const ConnectedServerButton = connect(mapStateToProps, mapDispatchToProps)(ServerButton)
export const ConnectedRendererButton = connect(mapRStateToProps, mapDispatchToProps)(RendererButton)
