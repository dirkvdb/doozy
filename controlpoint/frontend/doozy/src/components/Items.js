import React from 'react';

import {GridList, GridTile} from 'material-ui/GridList';

export default class Items extends React.Component {
    constructor(props) {
        super(props);

        this.state = {
            gridcols: 2,
        };
    }

    render() {
        return (
            <GridList
                cellHeight={180}
                cols={this.state.gridcols}
            >
                {this.props.items.map((item) => {
                    return <GridTile key={item.id} title={item.title} onTouchTap={() => this.props.onSelected(item)}>
                               <img src={item.thumbnailurl} role="presentation" />
                           </GridTile>;
                })}
            </GridList>
        );
    }
}

Items.propTypes = {
    items: React.PropTypes.arrayOf(
        React.PropTypes.shape({
            id: React.PropTypes.string.isRequired,
            title: React.PropTypes.string.isRequired,
        })
    ).isRequired,
    onSelected: React.PropTypes.func.isRequired
}
