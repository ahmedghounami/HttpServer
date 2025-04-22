var imgs = [
    'https://images.pexels.com/photos/2387966/pexels-photo-2387966.jpeg?auto=compress&cs=tinysrgb&w=1260&h=750&dpr=2',
    'https://images.pexels.com/photos/3762284/pexels-photo-3762284.jpeg?auto=compress&cs=tinysrgb&w=1260&h=750&dpr=2',
    'https://images.pexels.com/photos/4275890/pexels-photo-4275890.jpeg?auto=compress&cs=tinysrgb&w=1260&h=750&dpr=2',
    'https://images.pexels.com/photos/614484/pexels-photo-614484.jpeg?auto=compress&cs=tinysrgb&w=1260&h=750&dpr=2',
    'https://images.pexels.com/photos/460775/pexels-photo-460775.jpeg?auto=compress&cs=tinysrgb&w=1260&h=750&dpr=2',
    'https://images.pexels.com/photos/134058/pexels-photo-134058.jpeg?auto=compress&cs=tinysrgb&w=1260&h=750&dpr=2',
    'https://images.pexels.com/photos/17514561/pexels-photo-17514561/free-photo-of-eau-building-batiment-immeuble.jpeg?auto=compress&cs=tinysrgb&w=1260&h=750&dpr=2',
    'https://images.pexels.com/photos/1797100/pexels-photo-1797100.jpeg?auto=compress&cs=tinysrgb&w=1260&h=750&dpr=2',
    'https://images.pexels.com/photos/5409363/pexels-photo-5409363.jpeg?auto=compress&cs=tinysrgb&w=1260&h=750&dpr=2',
]

var med = 4;
function putImages() {
let img = document.querySelector('.pic_holder');
let b_imgs = document.querySelectorAll('.foot_img');
    img.style.backgroundImage = `url(${imgs[4 % imgs.length]})`;
    for (let j = 0; j < 8; j++) {
        i = j % imgs.length;
        b_imgs[j].style.backgroundImage = `url(${imgs[i]})`;
        b_imgs[j].onclick = function() {
            let t = j;
            while (j != med) {
                if (j < med) {
                    left_push(t);
                    j++;
                }
                else {
                    right_push(t);
                    j--;
                    j = (j + 8) % 8;
                }
            }
        }
        i++;
    }
}
function right_push(t){
    if (t == undefined) {
        t = med + 1;
    }
    let sel = document.querySelector('.select');
    imgs.push(imgs.shift());
    setTimeout(() => {
        sel.style.transform = 'scale(1)';
        sel.style.boxShadow = '0 0 0 0';
        sel.style.margin = '0';
        sel.addEventListener('transitionend', function handler() {
            sel.removeEventListener('transitionend', handler);
                for (let i = 0; i < 8; i++) {
                animations(document.getElementById('img' + i), t);
            }
            putImages();
        });
        setTimeout(() => {
            sel.style.transform = 'scale(1.5)';
            sel.style.boxShadow = '0 0 1rem 0.5rem #9d9d9d';
            sel.style.margin = '0 7rem';
        }, 800);
    }, 300);
    resetinterval();
}
function left_push(t){
    if (t == undefined) {
        t = med - 1;
    }
    let sel = document.querySelector('.select');
    imgs.unshift(imgs.pop());
    setTimeout(() => {
        sel.style.transform = 'scale(1)';
        sel.style.boxShadow = '0 0 0 0';
        sel.style.margin = '0';
        sel.addEventListener('transitionend', function handler() {
            sel.removeEventListener('transitionend', handler);
                for (let i = 0; i < 8; i++) {
                    animations(document.getElementById('img' + i), t);
            }
            putImages();
        });
        setTimeout(() => {
            sel.style.transform = 'scale(1.5)';
            sel.style.boxShadow = '0 0 1rem 0.5rem #9d9d9d';
            sel.style.margin = '0 7rem';
        }, 800);
    }, 300);
    resetinterval();
}
function animations(element, position){
    console.log(position);
    position = position - med;
    position *= 30;
    element.animate([
        {
            offset: 0,
            transform: "translateX(" + position + "rem)"
        },
        {
            offset: 1,
            transform: "translateX(0rem)"
        }
    ],{				 
        duration: 300,
        easing: 'linear',
        delay: 0,
        iterations: 1,
        direction: 'normal',
        fill: 'none'
    });
}
let inter = setInterval(right_push, 7000);
function resetinterval(){
    clearInterval(inter);
    inter = setInterval(right_push, 7000);
}
putImages();