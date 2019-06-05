var path = require('path');
var webpack = require('webpack');
const UglifyJsPlugin = require('uglifyjs-webpack-plugin');
module.exports = {
  entry: './main.js',
  output: {
    path: path.resolve(__dirname, '../Tmp/WEB'),
    publicPath: '/',
    filename: 'openZmeter.js'
  },
  plugins: [
    new webpack.ProvidePlugin({
    $: "jquery",
    jQuery: "jquery",
    "window.jQuery": "jquery"
    })
  ],
  module: {
    rules: [
      {
        test: /\.css$/,
        use: [
          'vue-style-loader',
          'css-loader'
        ]
      },      {
        test: /\.vue$/,
        loader: 'vue-loader',
        options: {
          loaders: {
          }
          // other vue-loader options go here
        }
      },
      {
        test: /\.js$/,
        loader: 'babel-loader',
        exclude: /node_modules/
      },
      {
        test: /\.(png|svg|woff2|ttf|eot|woff)$/,  
        use: [{
            loader: 'url-loader',
            options: {
              limit: 80000 // Convert images < 80kb to base64 strings
            }
          }]
      }
    ]
  },
  resolve: {
    alias: {
      'Components': path.resolve(__dirname, 'Components'),
      'JS': path.resolve(__dirname, 'JS'),
      'CSS': path.resolve(__dirname, 'CSS'),
      'Imgs': path.resolve(__dirname, 'Imgs'),
      'vue$': 'vue/dist/vue.esm.js'
    },
    extensions: ['*', '.js', '.vue', '.json']
  },
  devServer: {
    historyApiFallback: true,
    noInfo: true,
    overlay: true,
    proxy: {
      '/':             { target: 'http://192.168.8.10:80'},
      '/Login':        { target: 'http://192.168.8.10:80'},
      '/LogIn':        { target: 'http://192.168.8.10:80'},
      '/Logout':       { target: 'http://192.168.8.10:80'},
      '/getSeries':    { target: 'http://192.168.8.10:80'},
      '/getSeriesNow': { target: 'http://192.168.8.10:80'},
      '/getAnalyzers': { target: 'http://192.168.8.10:80'},
      '/getPrices':    { target: 'http://192.168.8.10:80'},
      '/getDevices':   { target: 'http://192.168.8.10:80'},
      '/startDevice':  { target: 'http://192.168.8.10:80'},
      '/stopDevice':   { target: 'http://192.168.8.10:80'},
      '/setDevices':   { target: 'http://192.168.8.10:80'},
      '/getTariffs':   { target: 'http://192.168.8.10:80'},
      '/setTariffs':   { target: 'http://192.168.8.10:80'},
      '/delTariffs':   { target: 'http://192.168.8.10:80'},
      '/getEvents':    { target: 'http://192.168.8.10:80'},
      '/getNetworks':  { target: 'http://192.168.8.10:80'},
      '/getDevice':    { target: 'http://192.168.8.10:80'},
      '/getBluetooth': { target: 'http://192.168.8.10:80'},
      '/setBluetooth': { target: 'http://192.168.8.10:80'},
      '/getUser':      { target: 'http://192.168.8.10:80'},
      '/setUser':      { target: 'http://192.168.8.10:80'}
    }
  },
  performance: {
    hints: false
  },
  devtool: '#eval-source-map'
};

if (process.env.NODE_ENV === 'production') {
  module.exports.devtool = '#source-map';
  // http://vue-loader.vuejs.org/en/workflow/production.html
  module.exports.plugins = (module.exports.plugins || []).concat([
    new webpack.DefinePlugin({
      'process.env': {
        NODE_ENV: '"production"'
      }
    }),
    new UglifyJsPlugin({
      'uglifyOptions': {
          compress: { warnings: false },
          sourceMap: true
      }
    }),
    new webpack.LoaderOptionsPlugin({
      minimize: true
    })
  ]);
}
