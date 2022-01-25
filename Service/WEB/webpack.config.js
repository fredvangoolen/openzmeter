var path = require('path');
var webpack = require('webpack');
const VueLoaderPlugin = require('vue-loader/lib/plugin');
module.exports = {
  entry: './Index.js',
  output: {
    path: path.resolve(__dirname, '../Tmp'),
    publicPath: '/',
    filename: 'openZmeter.js'
  },
  plugins: [
    new webpack.IgnorePlugin(/^\.\/locale$/, /moment$/),
    new VueLoaderPlugin(),
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
      }, {
        test: /\.vue$/,
        loader: 'vue-loader'
      }, {
        resourceQuery: /blockType=i18n/,
        type: 'javascript/auto',
        loader: '@intlify/vue-i18n-loader'
      }, {
        test: /\.js$/,
        exclude: /node_modules/,
        loader: 'babel-loader',
        options: {
          compact: true,
          plugins: ['@babel/plugin-syntax-dynamic-import']
        }
      }, {
        test: /\.(png|svg|woff2|ttf|eot|woff)$/i,
        use: [{
          loader: 'url-loader',
          options: {
            limit: 800000,
            esModule: false
          }
        }]
      }
    ]
  },
  resolve: {
    alias: {
      'Components': path.resolve(__dirname, 'Components'),
      'Views': path.resolve(__dirname, 'Views'),
      'App': path.resolve(__dirname, 'App'),
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
      '*': { target: 'http://localhost:8081/'}
    }
  },
  performance: {
    hints: false
  },
  devtool: '#eval-source-map'
};

if(process.env.NODE_ENV === 'production') {
  module.exports.devtool = false;
  module.exports.resolve.alias['vue$'] =  'vue/dist/vue.min.js';
  module.exports.plugins = (module.exports.plugins || []).concat([
    new webpack.DefinePlugin({
      'process.env': {
        NODE_ENV: '"production"'
      }
    }),
    new webpack.LoaderOptionsPlugin({
      minimize: true
    })
  ]);
}
