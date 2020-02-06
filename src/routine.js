

/* @author
 *
 * @3bute <fiedelferro@gmail.com>
 *
 */

var ax
  , ay
  , w
  , h
  , scl
  , cvs
  , degrad
  , dev0
  , dev1
  , dev3
  , dev4
  , dev5
  , iter
  , iPressed
  , oldMouseX
  , oldMouseY
  , dragged
  , prec
  , jPo
  , currentX
  , currentY
  , coords      = []
  , hold        = false
  , closed      = false
  , sat         = 126
  , Hue         = 170
  , Value       = 255
  , zoomed      = 0
  , midbutton   = false
  , prevColors  = null
  , bw          = false
  , dark        = false
  , workers     = 8
  , busy        = false
  , stop        = false
  , magnif      = false
  , h0          = 0
  , w0          = 0
  , proc        = []
  , P           = false
  , julia       = false
  , center      = null
  , done        = 0
  , destination = false;

function setup() {
  console.log('Width: ' + innerWidth + '\nHeight: ' +  innerHeight);
  cvs = createCanvas(windowWidth, windowHeight);
  h0 = height;
  w0 = width;
  scl = 2;
  w = width / scl;
  h = height / scl;
  iter = 100;
  degrad = 10;
  coords.push({ x0: -3, x1: 3 });
  let crd = coords[0];
  let xlen = crd.x1 - crd.x0;
  let ylen = (xlen * height) / width;
  crd.y1 = -ylen / 2;
  crd.y0 = ylen / 2;
  createControls();
  dropSet();
  setTimeout(() => (dev3.style.display = "none"), 20000);
}


function xypoints(x, y) {
  let crd = coords[coords.length-1];
  if (!P){
    x = parseFloat(x);
    y = parseFloat(y);
    let dtcx = x - (crd.x1 + crd.x0)/2,
        dtcy = y - (crd.y0 + crd.y1)/2;
    crd.x0 += dtcx;
    crd.x1 += dtcx;
    crd.y0 += dtcy;
    crd.y1 += dtcy;
    dropSet();
  }else{
    let dtcx = x.minus(crd.x1.plus(crd.x0).div(2)),
        dtcy = y.minus(crd.y0.plus(crd.y1).div(2));
    crd.x0 = crd.x0.plus(dtcx);
    crd.x1 = crd.x1.plus(dtcx);
    crd.y0 = crd.y0.plus(dtcy);
    crd.y1 = crd.y1.plus(dtcy);
    dropSetP();
  }
}

function getCenter() {
  let ctr = { }
    , crd = coords[coords.length-1];

  if (!P) {
    ctr.x = (crd.x0 + crd.x1)/2;
    ctr.y = (crd.y0 + crd.y1)/2;
  }else{
    ctr.x = crd.x0.plus(crd.x1).div(2);
    ctr.y = crd.y0.plus(crd.y1).div(2);
  }
  return ctr;
}

function zoom(i) {
  let crd = coords[coords.length -1]
  let ctr = (center) ?
    center : getCenter();
  if (!P) {
    ctr.xd = (crd.x1 - crd.x0)/i;
    ctr.yd = (crd.y0 - crd.y1)/i;
    crd.x0 = ctr.x - ctr.xd;
    crd.x1 = ctr.x + ctr.xd;
    crd.y0 = ctr.y + ctr.yd;
    crd.y1 = ctr.y - ctr.yd;
    if (Math.abs((crd.x1 - crd.x0)/(crd.y0 - crd.y1) - width/height) > 0.01) {
      switchP(true);
      alert('switched to high precision, the speed will drop significantly :\(');
      return ;
    }
    dropSet();
  }else{
    let crd = coords[coords.length-1];
    coords.push({ x0: new BigNumber(crd.x0),
                  x1: new BigNumber(crd.x1),
                  y0: new BigNumber(crd.y0),
    		          y1: new BigNumber(crd.y1)});
    crd = coords[coords.length-1];
    var p = crd.x1.toString().length;
    setPrecision(p);
    ctr.xd = crd.x1.minus(crd.x0).div(i);
    ctr.yd = crd.y0.minus(crd.y1).div(i);

    //achtung, inconvenience: elements have 0 index
    var crd_x0 = ctr.x0.minus(ctr.xd)
      , crd_x1 = ctr.x0.plus(ctr.xd)
      , crd_y0 = ctr.y0.plus(ctr.yd)
      , crd_y1 = ctr.y0.minus(ctr.yd)

    coords.push({x0: crd_x0, x1: crd_x1, y0: crd_y0, y1: crd_y1});
    let delta =  crd.x1.minus(crd.x0).div(crd.y0.minus(crd.y1)).minus(width/height).toFixed(3);
    if (Math.abs(Number(delta))>0.001 ) {
      prec+=2;
      setPrecision(prec);
    }
    dropSetP();
  }
}

function switchP(draw) {
  if (!P) {
    degrad = 30;
    workers = 64;
    if (!prec) prec = 17;
    setupP(draw);
  }else{
    degrad = 10;
    workers = 8;
    let crd = coords[coords.length-1];
    coords.push({
      x0: Number(crd.x0.toFixed(16)),
      x1: Number(crd.x1.toFixed(16)),
      y0: Number(crd.y0.toFixed(16)),
      y1: Number(crd.y1.toFixed(16))
    });
    createControls();
    dropSet();
  }
  P = !P;
}

function setPrecision(p){
  BigNumber.set({DECIMAL_PLACES:p})
  prec = p;
}

function stopCalc(){stop = true}
function makeSet() {
  destination = false;
  if (julia&&!jPo) {
    jPo = {};
    jPo.x = currentX;
    jPo.y = currentY;
    dropSet();
    return ;
  }
  zoomed += scl;

  let crd = coords[coords.length - 1]
    , xstt = Number(crd.x0)
    , xend = Number(crd.x1)
    , ystt = Number(crd.y0)
    , yend = 0
    , h = height / scl
    , w = width / scl
    , xlen = Number(crd.x1) - Number(crd.x0)
    , ylen = (xlen * h) / w;

  yend = ystt - ylen;
  crd.y1= yend;

  xstt = map(ax, 0, width, Number(crd.x0), Number(crd.x1));
  ystt = map(ay, 0, height, Number(crd.y0), Number(crd.y1));
  xend = map(ax + w, 0, width, Number(crd.x0), Number(crd.x1));
  yend = map(ay + h, 0, height, Number(crd.y0), Number(crd.y1));

  if (Math.abs((xend - xstt)/(ystt - yend) - width/height) > 0.01) {
    switchP(true);
    alert('switched to high precision, the speed will drop significantly :\(');
    return ;
  }

  coords.push({ x0: xstt, y0: ystt, x1: xend, y1: yend });
  dropSet();
}

function dropSet() {
  getPoints(
    iter,
    Number(coords[coords.length - 1].x0),
    Number(coords[coords.length - 1].y0),
    Number(coords[coords.length - 1].x1),
    Number(coords[coords.length - 1].y1)
  );
}

function getPoints(it, xstt, ystt, xend, yend) {
  colorMode(RGB, 255);
  if (bw) {
    if (!dark) background(0);
    else background(255);
  } else {
    background(255 - 125*255/Value);
  }
  show();
  done = 0;
  getValues(it, xstt, ystt, xend, yend, width / degrad, height / degrad)
  .then((res)=>{
       poll('http://localhost:1010' + res);
  });
}

function poll(url, pcs) {
  console.log(url);
  fetch(url)
  .then((res)=>{
    return res.text()
  })
  .then((res)=>{
    var a = res.split(';')
      , x = 0
      , y = 0;
    for (var n = 0; n < a.length; ++n){
      if (bw) {
        colorMode(RGB, 255);
        let gradient = delta * 255;
        if (dark) fill(gradient);
        else fill(255 - gradient);
      } else {
        colorMode(HSB, 255);
        if (a[n]==0) {
          colorMode(RGB);
          fill(125);
        }else{
          colorMode(HSB);
          let hue = a[n] / 100 * Hue;
          fill(hue, sat, Value);
        }
      }
      noStroke();
      rect(x * degrad, y * degrad, degrad, degrad);
      ++y;
      if (n % parseInt(height/degrad) == 0){
        ++x;
        y = 0;
      }
    }
    unshow();
    //if (done==(4 - 1)) return;
    //else {
    //  done++;
    //  setTimeout(poll(url), 100);
    //}
  })
}

function getValues(it, xstt, ystt, xend, yend, wi, he){
  return new Promise((resolve, reject)=>{
    fetch('http://localhost:1010/?it=' + it
        + '&xstt=' + xstt
        + '&ystt=' + ystt
        + '&xend=' + xend
        + '&yend=' + yend
        + '&wi=' + parseInt(wi)
        + '&he=' + parseInt(he)
        + '&bound=' + 2
        + '&zoom=' + 1)
    .then((res)=>{
      return res.text()
    })
    .then((res)=>{
      console.log(res);
      resolve(res);
    })
  })
}

function setupP(draw) {
  BigNumber.set({ DECIMAL_PLACES: prec });
  let crd = coords[coords.length-1];
  coords.push({ x0: new BigNumber(crd.x0),
	              x1: new BigNumber(crd.x1),
                y0: new BigNumber(crd.y0),
  		          y1: new BigNumber(crd.y1)});
  createControlsP();
  if (draw) {
    dropSetP();
  }
}

function makeSetP() {
  destination = false;
  if (julia&&!jPo) {
    jPo = {};
    jPo.x = currentX;
    jPo.y = currentY;
    dropSetP();
    return ;
  }
  zoomed += scl;
  let crd = coords[coords.length - 1];
  crd.x0 = new BigNumber(crd.x0);
  crd.x1 = new BigNumber(crd.x1);
  crd.y0 = new BigNumber(crd.y0);
  crd.y1 = new BigNumber(crd.y1);
  let xstt = crd.x0;
  let xend = crd.x1;
  let ystt = crd.y0;
  let h = new BigNumber(height).div(scl),
      w = new BigNumber(width).div(scl);
  let xlen = crd.x1.minus(crd.x0);
  let ylen = xlen.times(h).div(w);
  let yend = ystt.minus(ylen);
  crd.y1 = yend;
  xstt = mapFloat(ax, 0, width, crd.x0, crd.x1);
  ystt = mapFloat(ay, 0, height, crd.y0, crd.y1);
  xend = mapFloat(new BigNumber(ax).plus(w), 0, width, crd.x0, crd.x1);
  yend = mapFloat(new BigNumber(ay).plus(h), 0, height, crd.y0, crd.y1);
  let delta =  xend.minus(xstt).div(ystt.minus(yend)).minus(width/height).toFixed(4);
  if (Math.abs(Number(delta))>0.001 ) {
    prec+=2;
    setPrecision(prec);
  }
  coords.push({ x0: xstt, y0: ystt, x1: xend, y1: yend });
  dropSetP();
}

function dropSetP() {
  let crd = coords[coords.length-1];
  got_response = false;
  getPoints(
    iter,
    new BigNumber(crd.x0),
    new BigNumber(crd.y0),
    new BigNumber(crd.x1),
    new BigNumber(crd.y1)
  );
}

function mapFloat(a, b, c, d, e){
  let s0 = new BigNumber(b);
  let e0 = new BigNumber(c);
  let s1 = new BigNumber(d);
  let e1 = new BigNumber(e);
  let v = new BigNumber(a);
  return new BigNumber(v.minus(s0).div(e0.minus(s0)).times(e1.minus(s1)).plus(s1).toFixed(prec));
}
