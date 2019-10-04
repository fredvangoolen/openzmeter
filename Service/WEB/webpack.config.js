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
        options: {
          compact: true
        },
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
      '*': { target: 'http://10.8.0.135:80'}
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
          parallel: true,
          compress: { warnings: false },
          sourceMap: false
      }
    }),
    new webpack.LoaderOptionsPlugin({
      minimize: true
    })
  ]);
}
