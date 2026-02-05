// ========================================
// PORTFOLIO WEBSITE JAVASCRIPT
// ========================================

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

    carousel.addEventListener('wheel', (e) => {
        const trackHeight = track.scrollHeight;
        const visibleHeight = carousel.clientHeight;
        const minOffset = -(trackHeight - visibleHeight);

        if (trackHeight > visibleHeight) {
            offset -= e.deltaY;
            if (offset > 0) offset = 0;
            if (offset < minOffset) offset = minOffset;
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
        const minOffset = -(trackHeight - visibleHeight);

        if (trackHeight > visibleHeight) {
            offset += delta;
            if (offset > 0) offset = 0;
            if (offset < minOffset) offset = minOffset;
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
        const minOffset = -(trackHeight - visibleHeight);
        const stepSize = 40;

        if (trackHeight > visibleHeight) {
            if (e.key === 'ArrowDown' || e.key === 'PageDown' || e.key === ' ') {
                offset -= stepSize;
            } else if (e.key === 'ArrowUp' || e.key === 'PageUp') {
                offset += stepSize;
            } else if (e.key === 'Home') {
                offset = 0;
            } else if (e.key === 'End') {
                offset = minOffset;
            }

            if (offset > 0) offset = 0;
            if (offset < minOffset) offset = minOffset;
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

// Dynamically load external resources when needed
function lazyLoadResources() {
    // Load fonts or other resources on demand
    const link = document.createElement('link');
    link.rel = 'preload';
    link.as = 'font';
    link.href = 'https://fonts.googleapis.com/css2?family=Playfair+Display:wght@400;500;600;700&display=swap';
    link.crossOrigin = 'anonymous';
    document.head.appendChild(link);
}

// Call lazy load when page is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', lazyLoadResources);
} else {
    lazyLoadResources();
}

// ========================================
// CARTOON CAT CURSOR EFFECTS
// ========================================

// Create animated cursor follower
function initCartoonCursor() {
    // Create cursor element
    const cursor = document.createElement('div');
    cursor.id = 'cartoon-cursor';
    cursor.innerHTML = `
        <svg width="32" height="32" viewBox="0 0 32 32" xmlns="http://www.w3.org/2000/svg">
            <!-- Cartoon Cat Cursor -->
            <!-- Cat head -->
            <ellipse cx="16" cy="12" rx="10" ry="10" fill="#FF8C42" stroke="#2d3436" stroke-width="1.5"/>
            <!-- Left ear (pointy) - larger and more prominent -->
            <path d="M 9 2 L 6 8 L 12 8 Z" fill="#FF8C42" stroke="#2d3436" stroke-width="1.5"/>
            <!-- Right ear (pointy) - larger and more prominent -->
            <path d="M 23 2 L 26 8 L 20 8 Z" fill="#FF8C42" stroke="#2d3436" stroke-width="1.5"/>
            <!-- Left eye -->
            <ellipse cx="13" cy="11" rx="2" ry="2.5" fill="#2d3436"/>
            <!-- Right eye -->
            <ellipse cx="19" cy="11" rx="2" ry="2.5" fill="#2d3436"/>
            <!-- Nose (triangle) -->
            <path d="M 16 13 L 15 15 L 17 15 Z" fill="#2d3436"/>
            <!-- Mouth -->
            <path d="M 16 15 Q 14 17 12 16" stroke="#2d3436" stroke-width="1.5" fill="none" stroke-linecap="round"/>
            <path d="M 16 15 Q 18 17 20 16" stroke="#2d3436" stroke-width="1.5" fill="none" stroke-linecap="round"/>
            <!-- Whiskers -->
            <line x1="6" y1="12" x2="10" y2="12" stroke="#2d3436" stroke-width="1" stroke-linecap="round"/>
            <line x1="6" y1="14" x2="10" y2="14" stroke="#2d3436" stroke-width="1" stroke-linecap="round"/>
            <line x1="22" y1="12" x2="26" y2="12" stroke="#2d3436" stroke-width="1" stroke-linecap="round"/>
            <line x1="22" y1="14" x2="26" y2="14" stroke="#2d3436" stroke-width="1" stroke-linecap="round"/>
            <!-- Tail (pointing up like cursor) -->
            <path d="M 16 22 Q 12 28 8 26" stroke="#2d3436" stroke-width="2" fill="none" stroke-linecap="round"/>
        </svg>
    `;
    document.body.appendChild(cursor);

    // Add CSS for cursor follower
    const style = document.createElement('style');
    style.textContent = `
        #cartoon-cursor {
            position: fixed;
            width: 32px;
            height: 32px;
            pointer-events: none;
            z-index: 9999;
            transform: translate(-50%, -50%);
            transition: transform 0.1s ease-out;
            display: none;
        }
        
        #cartoon-cursor.active {
            display: block;
        }
        
        #cartoon-cursor.hover {
            transform: translate(-50%, -50%) scale(1.3);
        }
        
        #cartoon-cursor.click {
            transform: translate(-50%, -50%) scale(0.9);
        }
        
        body {
            cursor: none !important;
        }
        
        a, button, .portfolio-card, .experiment-card, .demo-btn {
            cursor: none !important;
        }
    `;
    document.head.appendChild(style);

    let mouseX = 0;
    let mouseY = 0;
    let cursorX = 0;
    let cursorY = 0;

    // Track mouse movement
    document.addEventListener('mousemove', (e) => {
        mouseX = e.clientX;
        mouseY = e.clientY;
        cursor.classList.add('active');
    });

    // Smooth cursor following
    function animateCursor() {
        cursorX += (mouseX - cursorX) * 0.15;
        cursorY += (mouseY - cursorY) * 0.15;
        cursor.style.left = cursorX + 'px';
        cursor.style.top = cursorY + 'px';
        requestAnimationFrame(animateCursor);
    }
    animateCursor();

    // Hover effects
    const hoverElements = document.querySelectorAll('a, button, .portfolio-card, .experiment-card, .demo-btn');
    hoverElements.forEach(el => {
        el.addEventListener('mouseenter', () => {
            cursor.classList.add('hover');
        });
        el.addEventListener('mouseleave', () => {
            cursor.classList.remove('hover');
        });
    });

    // Click effect
    document.addEventListener('mousedown', () => {
        cursor.classList.add('click');
    });
    document.addEventListener('mouseup', () => {
        cursor.classList.remove('click');
    });

    // Hide cursor when mouse leaves window
    document.addEventListener('mouseleave', () => {
        cursor.classList.remove('active');
    });
    document.addEventListener('mouseenter', () => {
        cursor.classList.add('active');
    });
}

// Initialize cursor on page load
document.addEventListener('DOMContentLoaded', initCartoonCursor);

