var path = require('path')
var webpack = require('webpack')
const UglifyJsPlugin = require('uglifyjs-webpack-plugin')
module.exports = {
  entry: './main.js',
  output: {
    path: path.resolve(__dirname, './dist'),
    publicPath: '/dist/',
    filename: 'openZmeter.js'
  },
  plugins: [
    new webpack.ProvidePlugin({
      $: "jquery",
      jQuery: "jquery"
    })
  ],
  module: {
    rules: [
      {
        test: /\.css$/,
        use: [
          'vue-style-loader',
          'css-loader'
        ],
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
              limit: 80000, // Convert images < 8kb to base64 strings
//              name: 'images/[hash]-[name].[ext]'
            }
          }]
      }, 
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
      '/Logout': { target: 'http://localhost:8081'},
      '/getSeries': { target: 'http://localhost:8081'},
      '/getSeriesNow': { target: 'http://localhost:8081'},
      '/getAnalyzers': { target: 'http://localhost:8081'},
      '/getPrices': { target: 'http://localhost:8081'},
      '/getDevices': { target: 'http://localhost:8081'},
      '/startDevice': { target: 'http://localhost:8081'},
      '/stopDevice': { target: 'http://localhost:8081'},
      '/setDevices': { target: 'http://localhost:8081'},
      '/getTariffs': { target: 'http://localhost:8081'},
      '/setTariffs': { target: 'http://localhost:8081'},
      '/delTariffs': { target: 'http://localhost:8081'},
      '/getEvents': { target: 'http://localhost:8081'},
      '/Login': { target: 'http://localhost:8081'}
    }
  },
  performance: {
    hints: false
  },
  devtool: '#eval-source-map'
}

if (process.env.NODE_ENV === 'production') {
  module.exports.devtool = '#source-map'
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
  ])
}
