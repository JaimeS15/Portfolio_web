// ========================================
// PORTFOLIO WEBSITE JAVASCRIPT
// ========================================

// ── Boot Loader ──────────────────────────
(function initBootLoader() {
    const loader = document.getElementById('boot-loader');
    if (!loader) return;

    // Lock scroll while loader is active
    document.body.style.overflow = 'hidden';

    const header   = loader.querySelector('.boot-header');
    const divider1 = loader.querySelector('.boot-divider');
    const rows     = loader.querySelectorAll('.boot-row');
    const divider2 = loader.querySelector('.boot-divider-2');
    const progRow  = loader.querySelector('.boot-progress-row');
    const barFill  = document.getElementById('bootBarFill');
    const pctEl    = document.getElementById('bootPct');
    const access   = document.getElementById('bootAccess');

    // Build ordered reveal sequence
    const sequence = [header, divider1, ...rows, divider2];
    const lineGap  = 120; // ms between each line

    sequence.forEach((el, i) => {
        setTimeout(() => el.classList.add('boot-visible'), 80 + i * lineGap);
    });

    // Progress row appears after all lines
    const progStart = 80 + sequence.length * lineGap + 60;
    setTimeout(() => {
        progRow.classList.add('boot-visible');

        // Kick off bar fill after a tick
        setTimeout(() => {
            barFill.style.width = '100%';

            // Count percentage in sync with bar (~700ms, 14 steps → ~50ms each)
            let count = 0;
            const step = Math.ceil(100 / 14);
            const ticker = setInterval(() => {
                count = Math.min(count + step, 100);
                pctEl.textContent = count + '%';
                if (count >= 100) clearInterval(ticker);
            }, 50);
        }, 60);
    }, progStart);

    // "ACCESS GRANTED" flashes on when bar is full
    const accessTime = progStart + 60 + 700 + 120;
    setTimeout(() => access.classList.add('boot-visible'), accessTime);

    // Wipe loader off screen
    setTimeout(() => {
        loader.classList.add('boot-exit');
        document.body.style.overflow = '';
        setTimeout(() => loader.remove(), 400);
    }, accessTime + 450);
})();

// Navbar scroll effect - hide on scroll down, show on scroll up
let lastScrollTop = 0;

window.addEventListener('scroll', () => {
    const navbar = document.querySelector('.navbar');
    const scrollTop = window.pageYOffset || document.documentElement.scrollTop;
    
    // Add scrolled class for styling
    if (scrollTop > 50) {
        navbar.classList.add('scrolled');
    } else {
        navbar.classList.remove('scrolled');
    }
    
    // Hide/show navbar based on scroll direction
    if (scrollTop > lastScrollTop && scrollTop > 100) {
        // Scrolling down - hide navbar
        navbar.classList.add('navbar-hidden');
    } else {
        // Scrolling up - show navbar
        navbar.classList.remove('navbar-hidden');
    }
    
    lastScrollTop = scrollTop <= 0 ? 0 : scrollTop; // For Mobile or negative scrolling
}, false);

// Smooth scroll for navigation links
document.querySelectorAll('a[href^="#"]').forEach(anchor => {
    anchor.addEventListener('click', function (e) {
        e.preventDefault();
        const target = document.querySelector(this.getAttribute('href'));
        if (target) {
            const offset = 80;
            const targetPosition = target.offsetTop - offset;
            window.scrollTo({
                top: targetPosition,
                behavior: 'smooth'
            });
        }
    });
});

// Intersection Observer for scroll animations
const observerOptions = {
    threshold: 0.1,
    rootMargin: '0px 0px -100px 0px'
};

const observer = new IntersectionObserver(function(entries) {
    entries.forEach(entry => {
        if (entry.isIntersecting) {
            entry.target.classList.add('fade-in-on-scroll');
            observer.unobserve(entry.target);
        }
    });
}, observerOptions);

// Observe portfolio cards - but don't hide them initially
document.querySelectorAll('.portfolio-card').forEach((card, index) => {
    // Start visible, just add animation class when in view
    observer.observe(card);
});

// Add active class to navigation links on scroll
const sections = document.querySelectorAll('section[id]');
const navLinks = document.querySelectorAll('.nav-links a');

window.addEventListener('scroll', () => {
    let current = '';
    
    sections.forEach(section => {
        const sectionTop = section.offsetTop;
        const sectionHeight = section.clientHeight;
        if (window.pageYOffset >= sectionTop - 100) {
            current = section.getAttribute('id');
        }
    });

    navLinks.forEach(link => {
        link.classList.remove('active');
        if (link.getAttribute('href') === `#${current}`) {
            link.classList.add('active');
        }
    });
});

// Add hover effects to portfolio cards
document.querySelectorAll('.portfolio-card').forEach(card => {
    card.addEventListener('mouseenter', function() {
        const video = this.querySelector('video');
        if (video) {
            video.play();
        }
    });

    card.addEventListener('mouseleave', function() {
        const video = this.querySelector('video');
        if (video) {
            video.pause();
        }
    });
});

// ========================================
// HERO CAROUSEL AUTO-SCROLL
// ========================================

function initHeroCarousel() {
    const carousel = document.querySelector('.hero-carousel');
    const track = document.querySelector('.hero-carousel-track');

    if (!carousel || !track) return;

    let isHovering = false;
    let isUserScrolling = false;
    let idleTimeout = null;
    let lastTimestamp = null;
    let offset = 0;

    const pixelsPerSecond = 60; // auto-scroll speed

    function applyTransform() {
        track.style.transform = `translateY(${offset}px)`;
    }

    function markUserScrolling() {
        isUserScrolling = true;
        if (idleTimeout) {
            clearTimeout(idleTimeout);
        }
        idleTimeout = setTimeout(() => {
            isUserScrolling = false;
        }, 800);
    }

    function step(timestamp) {
        if (!lastTimestamp) {
            lastTimestamp = timestamp;
        }
        const deltaSeconds = (timestamp - lastTimestamp) / 1000;
        lastTimestamp = timestamp;

        const trackHeight = track.scrollHeight;
        const visibleHeight = carousel.clientHeight;
        const loopOffset = -(trackHeight / 2); // because we duplicated cards

        if (!isHovering && !isUserScrolling && trackHeight > visibleHeight) {
            offset -= pixelsPerSecond * deltaSeconds;
            if (offset <= loopOffset) {
                offset = 0;
            }
            applyTransform();
        }

        requestAnimationFrame(step);
    }

    carousel.addEventListener('mouseenter', () => {
        isHovering = true;
    });

    carousel.addEventListener('mouseleave', () => {
        isHovering = false;
    });

    // Clamp at top (first card); seamless loop only when scrolling down past the duplicate set
    function clampAndLoop() {
        const trackHeight = track.scrollHeight;
        const halfTrack = trackHeight / 2;
        if (offset > 0) offset = 0;
        if (offset <= -halfTrack) offset += halfTrack;
    }

    carousel.addEventListener('wheel', (e) => {
        const trackHeight = track.scrollHeight;
        const visibleHeight = carousel.clientHeight;

        if (trackHeight > visibleHeight) {
            offset -= e.deltaY;
            clampAndLoop();
            applyTransform();
        }

        markUserScrolling();
    }, { passive: true });

    carousel.addEventListener('touchstart', (e) => {
        carousel.__touchStartY = e.touches[0].clientY;
    }, { passive: true });

    carousel.addEventListener('touchmove', (e) => {
        const startY = carousel.__touchStartY;
        if (startY == null) return;

        const currentY = e.touches[0].clientY;
        const delta = currentY - startY;

        const trackHeight = track.scrollHeight;
        const visibleHeight = carousel.clientHeight;

        if (trackHeight > visibleHeight) {
            offset += delta;
            clampAndLoop();
            applyTransform();
        }

        carousel.__touchStartY = currentY;
        markUserScrolling();
    }, { passive: true });

    window.addEventListener('keydown', (e) => {
        if (!carousel.matches(':hover')) return;
        const keys = ['ArrowUp', 'ArrowDown', 'PageUp', 'PageDown', 'Home', 'End', ' '];
        if (!keys.includes(e.key)) return;

        const trackHeight = track.scrollHeight;
        const visibleHeight = carousel.clientHeight;
        const stepSize = 40;

        if (trackHeight > visibleHeight) {
            if (e.key === 'ArrowDown' || e.key === 'PageDown' || e.key === ' ') {
                offset -= stepSize;
            } else if (e.key === 'ArrowUp' || e.key === 'PageUp') {
                offset += stepSize;
            } else if (e.key === 'Home') {
                offset = 0;
            } else if (e.key === 'End') {
                offset = -(trackHeight - visibleHeight);
            }

            clampAndLoop();
            applyTransform();
        }

        markUserScrolling();
        e.preventDefault();
    });

    applyTransform();
    requestAnimationFrame(step);
}

// Mobile menu toggle (if needed in future)
function initMobileMenu() {
    const navToggle = document.querySelector('.nav-toggle');
    const navLinks = document.querySelector('.nav-links');

    if (navToggle) {
        navToggle.addEventListener('click', function() {
            navLinks.classList.toggle('active');
        });

        // Close menu when link is clicked
        document.querySelectorAll('.nav-links a').forEach(link => {
            link.addEventListener('click', function() {
                navLinks.classList.remove('active');
            });
        });
    }
}

// Initialize on page load
document.addEventListener('DOMContentLoaded', function() {
    initMobileMenu();
    initHeroCarousel();
    
    // Add animation to elements with animation classes
    const animatedElements = document.querySelectorAll('[class*="animate"]');
    animatedElements.forEach(el => {
        observer.observe(el);
    });

    // Page transition: fade out on same-site link navigation
    const transitionDuration = 300; // ms, keep in sync with CSS
    const links = document.querySelectorAll('a[href]');

    links.forEach(link => {
        const href = link.getAttribute('href');
        if (!href) return;

        // Skip in-page anchors, mailto/tel, and new tab links
        if (href.startsWith('#') || href.startsWith('mailto:') || href.startsWith('tel:') || link.target === '_blank') {
            return;
        }

        link.addEventListener('click', function(e) {
            // Respect modifier keys (open in new tab/window)
            if (e.metaKey || e.ctrlKey || e.shiftKey || e.altKey) {
                return;
            }

            e.preventDefault();

            document.body.classList.add('page-fade-out');

            setTimeout(() => {
                window.location.href = href;
            }, transitionDuration);
        });
    });
});

// Prevent body scroll when scrolling animations are happening
let isAnimating = false;

window.addEventListener('wheel', (e) => {
    // Smooth scroll handling for better performance
    if (isAnimating) {
        e.preventDefault();
    }
}, { passive: true });

// Utility function to add scroll animations to any element
function addScrollAnimation(element, animationType = 'fadeInUp') {
    const elementObserver = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.style.animation = `${animationType} 0.8s ease-out forwards`;
                elementObserver.unobserve(entry.target);
            }
        });
    }, observerOptions);

    element.style.opacity = '0';
    elementObserver.observe(element);
}

// Add lazy loading for images
document.querySelectorAll('img').forEach(img => {
    if (!img.hasAttribute('loading')) {
        img.setAttribute('loading', 'lazy');
    }
});

// Performance optimization: Debounce scroll events
let scrollTimeout;
window.addEventListener('scroll', () => {
    clearTimeout(scrollTimeout);
    document.body.classList.add('is-scrolling');
    
    scrollTimeout = setTimeout(() => {
        document.body.classList.remove('is-scrolling');
    }, 150);
}, { passive: true });

// Back to top button functionality
const backToTopLink = document.querySelector('a[href="#back"]');
if (backToTopLink) {
    backToTopLink.addEventListener('click', (e) => {
        e.preventDefault();
        window.scrollTo({ top: 0, behavior: 'smooth' });
    });
}

// Log page load time for performance monitoring
window.addEventListener('load', () => {
    const perfData = window.performance.timing;
    const pageLoadTime = perfData.loadEventEnd - perfData.navigationStart;
    console.log('Page load time: ' + pageLoadTime + 'ms');
});

