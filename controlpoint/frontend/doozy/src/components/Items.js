import React from 'react';

import {GridList, GridTile} from 'material-ui/GridList';
import IconButton from 'material-ui/IconButton';
import {white} from 'material-ui/styles/colors';
import Play from 'material-ui/svg-icons/av/play-circle-outline';

export default class Items extends React.Component {
  constructor(props) {
    super(props);

    this.state = {
      gridcols: 2,
      cellHeight: 'auto',
    };
  }

  componentDidMount() {
    this.handleResize()
    window.addEventListener("resize", this.handleResize.bind(this));
  }

  handleResize () {
    let cols = Math.floor(window.innerWidth / 200)
    let cellHeight = Math.floor(window.innerWidth / cols);
    this.setState({gridcols: cols, cellHeight: cellHeight})
  }

  modifyUrl (url) {
    if (url.endsWith('?scale=org')) {
      // this is a twonky server, use optimal scale to avoid unnecessarily large images
      return url.replace('org', this.state.cellHeight + 'x' + this.state.cellHeight)
    }

    return url
  }

  render() {
    return (
      <GridList
        cellHeight={this.state.cellHeight}
        cols={this.state.gridcols}
      >
        {this.props.items.map((item) => {
          return <GridTile
                    key={item.id}
                    title={item.title}
                    subtitle={item.artist}
                    actionIcon={<IconButton>
                                  <Play color={white} onTouchTap={() => this.props.onSelected(item)}/>
                                </IconButton>
                    }
                 >
                   <img src={this.modifyUrl(item.thumbnailurl)} role="presentation"
                     onTouchTap={() => this.props.onSelected(item)}
                   />
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
